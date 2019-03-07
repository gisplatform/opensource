// Author: Cameron Norman https://launchpad.net/~cameronnemo
// See https://code.launchpad.net/~elementary-dev-community/audience/prevent-screen-timeout/+merge/262939

[CCode (cheader_filename = "X11/extensions/dpms.h")]
namespace DPMS
{
  [CCode (cname = "DPMSEnable")]
  X.Status enable(X.Display x);
  [CCode (cname = "DPMSDisable")]
  X.Status disable(X.Display x);
  [CCode (cname = "DPMSInfo")]
  X.Status info(X.Display x, out uint16 power_level, out char state);
}
