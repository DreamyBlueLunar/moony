#include "poller.h"
#include "../../net/channel/channel.h"

bool moony::poller::has_channel(moony::channel* chann) const {
    channel_map::const_iterator it = channels_.find(chann->fd());
    return it != channels_.end() && it->second == chann;
}
