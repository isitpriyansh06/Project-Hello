#include <stic.h>

#include <unistd.h> /* rmdir() */

#include <string.h> /* strcat() */

#include "../../src/cfg/config.h"
#include "../../src/compat/fs_limits.h"
#include "../../src/ui/ui.h"
#include "../../src/utils/fs.h"
#include "../../src/filelist.h"
#include "../../src/fops_misc.h"
#include "../../src/trash.h"

#include "utils.h"

char trash_dir[PATH_MAX + 1];
static char *saved_cwd;

SETUP()
{
	view_setup(&lwin);
	set_to_sandbox_path(lwin.curr_dir, sizeof(lwin.curr_dir));
	view_setup(&rwin);

	create_empty_file(SANDBOX_PATH "/file");
	saved_cwd = save_cwd();
	populate_dir_list(&lwin, 0);
	restore_cwd(saved_cwd);
	saved_cwd = save_cwd();

	cfg.use_trash = 1;
	make_abs_path(trash_dir, sizeof(trash_dir), SANDBOX_PATH, "trash", saved_cwd);
	trash_set_specs(trash_dir);
	lwin.dir_entry[0].marked = 1;
	(void)fops_delete(&lwin, 'a', 1);
}

TEARDOWN()
{
	view_teardown(&lwin);
	view_teardown(&rwin);
	restore_cwd(saved_cwd);
	assert_success(rmdir(trash_dir));
}

TEST(files_not_directly_in_trash_are_not_restored)
{
	trash_set_specs(lwin.curr_dir);

	strcat(lwin.curr_dir, "/trash");
	populate_dir_list(&lwin, 0);
	restore_cwd(saved_cwd);
	saved_cwd = save_cwd();

	lwin.dir_entry[0].marked = 1;
	(void)fops_restore(&lwin);

	assert_success(unlink(SANDBOX_PATH "/trash/000_file"));
}

TEST(generally_restores_files)
{
	strcpy(lwin.curr_dir, trash_dir);
	populate_dir_list(&lwin, 0);
	restore_cwd(saved_cwd);
	saved_cwd = save_cwd();

	lwin.dir_entry[0].marked = 1;
	(void)fops_restore(&lwin);

	assert_success(unlink(SANDBOX_PATH "/file"));
}

TEST(works_with_custom_view)
{
	char path[PATH_MAX + 1];
	make_abs_path(path, sizeof(path), SANDBOX_PATH, "trash/000_file", saved_cwd);

	flist_custom_start(&lwin, "test");
	flist_custom_add(&lwin, path);
	assert_true(flist_custom_finish(&lwin, CV_REGULAR, 0) == 0);

	lwin.dir_entry[0].marked = 1;
	(void)fops_restore(&lwin);

	assert_success(unlink(SANDBOX_PATH "/file"));
}

TEST(works_with_tree_view)
{
	assert_success(flist_load_tree(&lwin, lwin.curr_dir));

	lwin.dir_entry[1].marked = 1;
	(void)fops_restore(&lwin);

	assert_success(unlink(SANDBOX_PATH "/file"));
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
