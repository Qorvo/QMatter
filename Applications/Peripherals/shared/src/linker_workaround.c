#include "global.h"
#include "hal_user_license.h"
#if defined(GP_DIVERSITY_LOADED_USER_LICENSE)
const loaded_userlicense_t *(*force_keep_user_license)(void) = hal_get_loaded_user_license;
#else
const userlicense_t *(*force_keep_user_license)(void) = hal_get_user_license;
#endif
#if defined(GP_DIVERSITY_JUMPTABLES)
#include "gpJumpTables_DataTable.h"
const void* forceDataJumpTableInclude = &JumpTables_DataTable;
#endif // GP_DIVERSITY_JUMPTABLES
