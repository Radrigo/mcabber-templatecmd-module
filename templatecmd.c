
/* Copyright 2009 Myhailo Danylenko
 *
 * This file is part of mcabber-templatecmd
 *
 * mcabber-templatecmd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include <mcabber/compl.h>
#include <mcabber/commands.h>
#include <mcabber/utils.h>
#include <mcabber/logprint.h>
#include <mcabber/modules.h>

#include "config.h"

void tcmd_init (void);
void tcmd_uninit (void);

#define DESCRIPTION ( "Templated commands\nProvides command /templatecmd (and any commands, you define with it)" )

module_info_t info_templatecmd = {
	.branch      = MCABBER_BRANCH,
	.api         = MCABBER_API_VERSION,
	.version     = PROJECT_VERSION,
	.description = DESCRIPTION,
	.requires    = NULL,
	.init        = tcmd_init,
	.uninit      = tcmd_uninit,
	.next        = NULL,
};

typedef struct {
	gchar    *name;
#ifdef MCABBER_API_HAVE_CMD_ID
	gpointer  id;
#endif
	gchar    *template;
	int       maxarg;
	gboolean  stararg;
} tcmd_t;

#ifdef MCABBER_API_HAVE_CMD_ID
static gpointer tcmd_cmid = NULL;
static gboolean tcmd_set_safe = FALSE;
#endif

static GSList *template_commands = NULL;

static void tcmd_callback (char *arg, gpointer udata)
{
	tcmd_t   *tcmd     = (tcmd_t *) udata;
	char    **args     = NULL;
	GString  *command  = g_string_new (NULL);
	char     *template = tcmd -> template;
	char     *p;

	if (tcmd -> maxarg > 0)
		args = split_arg (arg, tcmd -> maxarg, TRUE);

	for (p = template; *p; ++p) {
		if (*p != '$' || !*(p + 1) || !strchr ("*$0123456789", *(p + 1))) {
			g_string_append_c (command, *p);
			continue;
		}

		++p;

		if (*p == '*')
			g_string_append (command, arg);
		else if (*p == '$')
			g_string_append_c (command, '$');
		else {
			int argnum = atoi (p);

			if (argnum == 0)
				g_string_append (command, tcmd -> name);
			else if (args[argnum - 1])
				g_string_append (command, args[argnum - 1]);

			while (*(p + 1) && strchr ("0123456789", *(p + 1)))
				++p;
		}
	}

	{
		char *cmd = g_string_free (command, FALSE);
		process_command (cmd, 1);
		g_free (cmd);
	}

	if (args)
		free_arg_lst (args);
}

static void do_templatecmd (char *arg)
{
	if (!*arg) { // list tcmds

		GSList *tel;

		for (tel = template_commands; tel; tel = tel -> next) {
			tcmd_t *tcmd = (tcmd_t *) tel -> data;
			scr_log_print (LPRINT_NORMAL, "Templatecmd %s = %s", tcmd -> name, tcmd -> template);
		}

	} else {

		char *nend = strchr (arg, '=');

		if (!nend) { // show tcmd

			GSList *tel;

			for (tel = template_commands; tel; tel = tel -> next) {
				tcmd_t *tcmd = (tcmd_t *) tel -> data;

				if (!strcmp (arg, tcmd -> name)) {
					scr_log_print (LPRINT_NORMAL, "Templatecmd %s = %s", tcmd -> name, tcmd -> template);
					return;
				}
			}

			scr_log_print (LPRINT_NORMAL, "No template with such name.");

		} else if (nend == arg) // error

			scr_log_print (LPRINT_NORMAL, "You must specify command name.");

		else { // new/modify tcmd

			GSList *tel;
			tcmd_t *template_command = NULL;
			char   *astart           = nend + 1;
			int     len;

			for (--nend; nend > arg && *nend == ' '; --nend);
			len = nend + 1 - arg;

			for (tel = template_commands; tel; tel = tel -> next) {
				tcmd_t *tcmd = (tcmd_t *) tel -> data;

				if (!strncmp (arg, tcmd -> name, len)) {
					template_command = tcmd;
					break;
				}
			}

			for (;*astart && *astart == ' '; ++astart);

			if (!*astart) { // delete tcmd
				if (template_command) {
#ifndef MCABBER_API_HAVE_CMD_ID
					cmd_del (template_command -> name);
#else
					if (template_command -> id)
						cmd_del (template_command -> id);
#endif
					template_commands = g_slist_remove (template_commands, template_command);
					g_free (template_command -> name);
					g_free (template_command -> template);
					g_slice_free (tcmd_t, template_command);
				}
				return;
			}

			if (!template_command) {
				template_command = g_slice_new (tcmd_t);
				template_command -> name = g_strndup (arg, len);
			} else
				g_free (template_command -> template);

			template_command -> template = g_strdup (astart);

			{
				int      maxarg  = -1;
				gboolean stararg = FALSE;

				for (astart = strchr (astart, '$'); astart; astart = strchr (astart, '$')) {
					++astart;

					if (!*astart)
						break;

					if (strchr ("0123456789", *astart)) {
						int anum = atoi (astart);

						if (maxarg < anum)
							maxarg = anum;

					} else if (*astart == '*')
						stararg = TRUE;
				}

				template_command -> maxarg  = maxarg;
				template_command -> stararg = stararg;
			}

			template_commands = g_slist_append (template_commands, template_command);

#ifndef MCABBER_API_HAVE_CMD_ID
			cmd_add (template_command -> name, "", 0, 0, (void (*) (char *arg)) tcmd_callback, template_command);
#else
			template_command -> id = cmd_add (template_command -> name, "", 0, 0, (void (*) (char *arg)) tcmd_callback, template_command);
#endif
		}
	}
}


void tcmd_init (void)
{
#ifndef MCABBER_API_HAVE_CMD_ID
	cmd_add ("templatecmd", "", COMPL_CMD, COMPL_CMD, do_templatecmd, NULL);
#else
	tcmd_cmid = cmd_add ("templatecmd", "", COMPL_CMD, COMPL_CMD, do_templatecmd, NULL);
	tcmd_set_safe = cmd_set_safe ("templatecmd", TRUE);
#endif
}

void tcmd_uninit (void)
{
	GSList *tel;

#ifndef MCABBER_API_HAVE_CMD_ID
	cmd_del ("templatecmd");
#else
	if (tcmd_cmid)
		cmd_del (tcmd_cmid);
	if (tcmd_set_safe)
		cmd_set_safe ("templatecmd", FALSE);
#endif

	for (tel = template_commands; tel; tel = tel -> next) {
		tcmd_t *template_command = (tcmd_t *) tel -> data;
#ifndef MCABBER_API_HAVE_CMD_ID
		cmd_del (template_command -> name);
#else
		if (template_command -> id)
			cmd_del (template_command -> id);
#endif
		g_free (template_command -> name);
		g_free (template_command -> template);
		g_slice_free (tcmd_t, template_command);
	}

	g_slist_free (template_commands);
}

/* vim: se ts=4 sw=4: */
