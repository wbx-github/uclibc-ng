/*
 * posix_spawn_file_actions_addchdir and _addfchdir, POSIX.1-2024.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>

#include "spawn_int.h"

static struct __spawn_action *
new_action(posix_spawn_file_actions_t *file_actions)
{
	if (file_actions->__used == file_actions->__allocated
	    && __posix_spawn_file_actions_realloc(file_actions) != 0)
		return NULL;
	return &file_actions->__actions[file_actions->__used];
}

int
posix_spawn_file_actions_addchdir(posix_spawn_file_actions_t *file_actions,
				  const char *path)
{
	struct __spawn_action *rec = new_action(file_actions);
	char *copy;

	if (rec == NULL)
		return ENOMEM;
	copy = strdup(path);
	if (copy == NULL)
		return ENOMEM;

	rec->tag = spawn_do_chdir;
	rec->action.chdir_action.path = copy;
	++file_actions->__used;
	return 0;
}
strong_alias(posix_spawn_file_actions_addchdir,
	     posix_spawn_file_actions_addchdir_np)

int
posix_spawn_file_actions_addfchdir(posix_spawn_file_actions_t *file_actions,
				   int fd)
{
	struct __spawn_action *rec;

	if (fd < 0)
		return EBADF;
	rec = new_action(file_actions);
	if (rec == NULL)
		return ENOMEM;

	rec->tag = spawn_do_fchdir;
	rec->action.fchdir_action.fd = fd;
	++file_actions->__used;
	return 0;
}
strong_alias(posix_spawn_file_actions_addfchdir,
	     posix_spawn_file_actions_addfchdir_np)
