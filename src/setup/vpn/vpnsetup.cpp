//#define LIBENCLOUD_DISABLE_TRACE  // disable heavy tracing
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/vpn/VpnSetup.h>

// Login/Logout are unnecessary when using Basic Auth, and logout doesn't kick
// out of VPN connection which is what we really would need
//#define LIBENCLOUD_SETUP_QCC_WITH_LOGIN

namespace libencloud {

/**
 * Unlike ECE/SECE Setup, the QCC Setup Module makes no automatic retries =>
 * errors are forwarded to user.
 */

VpnSetup::VpnSetup (Config *cfg)
    : SetupInterface(cfg)
{
    LIBENCLOUD_TRACE;
}

int VpnSetup::start ()
{
    LIBENCLOUD_TRACE;

    _clear();

    _vpnConfig.fromFile(_cfg->config.vpnConfPath.absoluteFilePath(), false);
    _vpnFallbackConfig.fromFile(_cfg->config.fallbackVpnConfPath.absoluteFilePath(), false);

    emit progress(Progress(tr("Using static Switchboard configuration"),
                           StateLast, getTotalSteps()));

    emit completed();

    return 0;
}

int VpnSetup::stop ()
{
    LIBENCLOUD_TRACE;

    _clear();
    return 0;
}

const VpnConfig *VpnSetup::getVpnConfig () const
{
    return &_vpnConfig;
}

const VpnConfig *VpnSetup::getFallbackVpnConfig () const
{
    return &_vpnFallbackConfig;
}

int VpnSetup::getTotalSteps() const
{
    return StateLast - StateFirst + 1;
}

// Clear all generated data
void VpnSetup::_clear ()
{
    _vpnConfig.clear();
    _vpnFallbackConfig.clear();
    return;
}

} // namespace libencloud
