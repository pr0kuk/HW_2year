#include "log.h"

int main()
{
    log_init(NULL);
    pr_err ("I'm here!");
    pr_warn("I'm here!");
    pr_info("I'm here!");
}
