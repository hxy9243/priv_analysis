/*
 * descript:
 * This is is a capability library wrapper for linux,
 * see more details, please refer to libcap and
 * <sys/capability.h> for more details.
 * Link with: -lcap
 *
 * author:  Hu Xiaoyu
 * created: Sep. 20 2014
 */


#ifndef __CAP_H__
#define __CAP_H__


#define CAP_NUM (CAP_LAST_CAP + 1)

#include <sys/capability.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>


/*
 * Raise the capability in the current list
 * for the current thread.
 * param: const int count - total number of capabilities for operation;
 *        const cap_value_t cap - capability values defined in
 *                                <linux/capability.h>
 *                                
 * return: 0 on success
 *         -1 on any error
 */
int priv_raise (const int count, ...);


/*
 * Raise the capability in the current list
 * for the current thread.
 * param: const int count - total number of capabilities for operation;
 *        const cap_value_t cap - capability values defined in
 *                                <linux/capability.h>
 *                                
 * return: 0 on success
 *         -1 on any error
 */
int priv_lower (const int count, ...);


/*
 * Raise the capability in the current list
 * for the current thread.
 * param: const int count - total number of capabilities for operation;
 *        const cap_value_t cap - capability values defined in
 *                                <linux/capability.h>
 *                                
 * return: 0 on success
 *         -1 on any error
 */
int priv_drop (const int count, ...);


/*
 * Drop all the capabilities in the current list
 * for the current thread.
 * return: 0 on success
 *         -1 on any error
 */
int priv_lowerall ();


/*
 * Print out the capability of the current process
 */
void print_cap();

#endif
