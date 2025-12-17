
#include "core/tx_notifier.h"


static tx_notify_fn tx_notify = nullptr;

void register_tx_notifier(tx_notify_fn fn)
{
    tx_notify = fn;
}

void notify_tx_ready()
{
    if (tx_notify)
        tx_notify();
}
