#include "hal.h"
#include "FreeRTOS.h"
#include "gpSched.h"
#include "task.h"

extern void Application_Init(void);

#ifdef GP_SCHED_EXTERNAL_MAIN
MAIN_FUNCTION_RETURN_TYPE MAIN_FUNCTION_NAME(void)
{
    HAL_INITIALIZE_GLOBAL_INT();

    // Hardware initialization
    HAL_INIT();

    HAL_ENABLE_GLOBAL_INT();

    gpSched_Init();

    /* Make sure to run the stack-intensive initialisation code from the scheduler task with larger stack */
    gpSched_ScheduleEvent(0, Application_Init);
    vTaskStartScheduler();
    return MAIN_FUNCTION_RETURN_VALUE;
}
#endif //GP_SCHED_EXTERNAL_MAIN
