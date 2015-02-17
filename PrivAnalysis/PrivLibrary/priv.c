/*
 * descript:
 * This is is a capability library wrapper for linux,
 * see more details, please refer to libcap and
 * <sys/capability.h> for more details.
 * 
 * dependency: libcap
 * link flag: -lcap
 *
 * author:  Kevin Hu
 * created: Sep. 20 2014
 */


#include "priv.h"


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
int priv_raise(const int count, ...)
{
  int i;

  cap_t cap;
  cap_value_t cap_v;
  cap_value_t *cap_v_list;

  va_list arglist;

  // alloca mem for cap_v_list, return error if count is oversized
  if (count > CAP_LAST_CAP) {
    return -1;
  }
  cap_v_list = (cap_value_t *)alloca(sizeof (cap_value_t) * count);

  // get the capability of the current process
  if ( (cap = cap_get_proc ()) == NULL ) {
    return -1;
  }

  // set cap_v_list to effective flag of capability variable
  va_start (arglist, count);
  for (i = 0; i < count; i ++) {
    cap_v = va_arg (arglist, int);
    cap_v_list[i] = cap_v;
  }
  va_end (arglist);

  // set the flag of cap to value in cap_v_list
  if ( cap_set_flag(cap,
                    CAP_EFFECTIVE,
                    count,
                    cap_v_list,
                    CAP_SET) != 0) {
    cap_free (cap);
    return -1;
  }

  // set the capability to current proc
  if ( cap_set_proc (cap) < 0) {
    cap_free(cap);
    return -1;
  }

  cap_free(cap);
  return 0;
}



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
int priv_lower(const int count, ...)
{
  int i;

  cap_t cap;
  cap_value_t cap_v;
  cap_value_t *cap_v_list;

  va_list arglist;

  // alloca mem for cap_v_list, return error if count is oversized
  if (count > CAP_LAST_CAP) {
    return -1;
  }
  cap_v_list = (cap_value_t *)alloca(sizeof(cap_value_t) * count);

  // get the capability of the current process
  if ( (cap = cap_get_proc ()) == NULL ) {
    return -1;
  }

  // set cap_v_list to effective flag of capability variable
  va_start (arglist, count);
  for (i = 0; i < count; i ++) {
    cap_v = va_arg(arglist, int);
    cap_v_list[i] = cap_v;
  }
  va_end (arglist);

  // set the flag of cap to value in cap_v_list
  if ( cap_set_flag(cap,
                    CAP_EFFECTIVE,
                    count,
                    cap_v_list,
                    CAP_CLEAR) != 0) {
    cap_free (cap);
    return -1;
  }

  // set the capability to current proc
  if ( cap_set_proc (cap) < 0){
    cap_free (cap);
    return -1;
  }

  cap_free (cap);
  return 0;
}



/*
 * Drop the capability in the current list
 * for the current thread.
 * param: const int count - total number of capabilities for operation;
 *        const cap_value_t cap - capability values defined in
 *                                <linux/capability.h>
 *                                
 * return: 0 on success
 *         -1 on any error
 */
int priv_remove(const int count, ...)
{
  int i;

  cap_t cap;
  cap_value_t cap_v;
  cap_value_t *cap_v_list;

  va_list arglist;

  // alloca mem for cap_v_list, return error if count is oversized
  if (count > CAP_LAST_CAP) {
    return -1;
  }
  cap_v_list = (cap_value_t *)alloca(sizeof (cap_value_t) * count);

  // get the capability of the current process
  if ( (cap = cap_get_proc ()) == NULL ) {
    return -1;
  }

  // set cap_v_list to effective flag of capability variable
  va_start (arglist, count);
  for (i = 0; i < count; i ++) {
    cap_v = va_arg(arglist, int);
    cap_v_list[i] = cap_v;
  }
  va_end(arglist);

  // set the flag of cap to value in cap_v_list
  if ( cap_set_flag(cap,
                    CAP_EFFECTIVE,
                    count,
                    cap_v_list,
                    CAP_CLEAR) != 0) {
    cap_free(cap);
    return -1;
  }

  if ( cap_set_flag(cap,
                    CAP_PERMITTED,
                    count,
                    cap_v_list,
                    CAP_CLEAR) != 0){
    cap_free(cap);
    return -1;
  }

  // set the capability to current proc
  if ( cap_set_proc (cap) < 0) {
    cap_free(cap);
    return -1;
  }

  cap_free (cap);
  return 0;
}


/*
 * Drop all the capabilities in the current list
 * for the current thread.
 * return: 0 on success
 *         -1 on any error
 */
int priv_lowerall ()
{
  int i;

  cap_t cap;
  cap_value_t cap_v;
  cap_value_t *cap_v_list;

  va_list arglist;

  // get the capability of the current process
  if ( (cap = cap_get_proc()) == NULL ) {
    return -1;
  }

  cap_v_list = (cap_value_t *)alloca(sizeof(cap_value_t) * CAP_NUM);

  // set cap_v_list to effective flag of capability variable
  for (i = 0; i < CAP_NUM; i ++) {
    cap_v_list[i] = i;
  }

  // set the flag of cap to value in cap_v_list
  if ( cap_set_flag(cap,
                    CAP_EFFECTIVE,
                    CAP_NUM,
                    cap_v_list,
                    CAP_CLEAR) != 0){
    cap_free (cap);
    return -1;
  }
  // set the capability to current proc
  if ( cap_set_proc(cap) < 0){
    cap_free(cap);
    return -1;
  }

  cap_free(cap);
  return 0;
}



/*
 * Print out the capability of the current process
 */
void print_cap()
{
    int i;
    cap_t cap;
    cap_flag_value_t value;
    cap = cap_get_proc();

    printf ("\nPERM\t");
    for (i = 0; i <= CAP_LAST_CAP; i ++){
        cap_get_flag (cap, i, CAP_PERMITTED, &value);
        printf ("%d", value == CAP_SET);
    }

    printf ("\nINHERIT\t");
    for (i = 0; i <= CAP_LAST_CAP; i ++){
        cap_get_flag (cap, i, CAP_INHERITABLE, &value);
        printf ("%d", value == CAP_SET);
    }

    printf ("\nEFFECT\t");
    for (i = 0; i <= CAP_LAST_CAP; i ++){
        cap_get_flag (cap, i, CAP_EFFECTIVE, &value);
        printf ("%d", value == CAP_SET);
    }
    printf("\n");

    cap_free(cap);
}
