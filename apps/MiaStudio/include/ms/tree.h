#ifndef MS_TREE_H
#define MS_TREE_H

/**
 * @file tree.h
 *
 * Manages the internal file tree.
 *
 * Internally its loading and saving a .json file
 * The structure is:
 *
 * {
 *   "tree": {
 *     "foo": {
 *       "?type": "DIR",
 *       "?time": 1234567,
 *       "bar": {
 *         "?type:" "IMG",
 *         "?time": 2344
 *         "?raw": 1239536
 *       }
 *     }
 *   }
 * }
 *
 * So in the tree object, each key without a ? at the beginning is a name of a path.
 * Can be a directory or a file itself (without children)
 * If foo is a directory, its nested files are simply all children without "?" at the beginning.
 *
 */

#include "ms/MSFile.h"

/**
 * Inits the file tree manager
 * @param parent
 */
void ms_tree_init(oobj parent);

/**
 * Returns the raw saved file, without type.
 * @param file MSFile object
 * @return real mia file path to the raw file, without type endings!
 * @note asserts main thread.
 *       only valid until the next ms_tree_path call!
 */
const char *ms_tree_raw_path(oobj file);

/**
 * @param file MSFile object
 * @return real mia file path to the image file
 * @note asserts main thread.
 *       asserts MSFile_type(file) == MSFile_type_IMG.
 *       only valid until the next ms_tree_path_img call!
 */
const char *ms_tree_img_path(oobj file);

/**
 * @param file MSFile object
 * @return real mia file path to the image thumb file
 * @note asserts main thread.
 *       asserts MSFile_type(file) == MSFile_type_IMG.
 *       only valid until the next ms_tree_img_thumb_path call!
 */
const char *ms_tree_img_thumb_path(oobj file);

/**
 * Updates the thumb image by loading the image in ms_tree_img_path and saving into ms_tree_img_thumb_path
 * @param file MSFile object
 * @note asserts main thread.
 *       asserts MSFile_type(file) == MSFile_type_IMG.
 *       invalidates both ms_tree_img_path and ms_tree_img_thumb_path
 */
void ms_tree_img_thumb_update(oobj file);

/**
 * Creates all MSFiles in a given dir path
 * @param parent to allocate on
 * @param file_dir dir to ls / create the files from, if ends with "/.." it will resolve in the parent dir
 * @return NULL terminated list of MSFiles, allocated on parent (including the list itself)
 * @note in ascending order. First entry is a ".." dir, to the parent directory (in case file_dir != "")
 */
MSFile **ms_tree_ls(oobj parent, const char *file_dir, osize *opt_out_num_files);



#endif //MS_TREE_H
