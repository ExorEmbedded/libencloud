Encloud Configuration
=====================

### General Info

libencloud's Config module gives client programmes (which may be a QCC Connect App, ECE/SECE device, ..)
read/write access to Connect Service configuration.

__IMPORTANT NOTE__: manual editing of the configuration file is discouraged! Most settings can be
configured dynamically via the Config REST API [see CONFIG.rst for usage].

The config file has the following default path:

    Windows         %ProgramFiles%/libencloud/etc/libencloud.json
    Mac OSX         /Applications/${ORGANIZATION}/${PRODUCTNAME}.app/ \
                        Contents/Resources/libencloud/etc/libencloud.json
    Linux           /etc/encloud/libencloud.json

### Syntax

A full-fledged sample with comments follows (comments are not valid JSON, so remember to strip them and pay attention to
strict syntax with commas and brackets):

    {
      # [General Config]
      "timeout" : 90,       # maximum time for cloud connections (in seconds, default=60)
      "autoretry" : true,   # whether to retry upon error (default=false)
      "decongest" : true,   # whether to apply a random wait to avoid congenstion (default=false for QCC, true for ECE)
      "bind" : "all",       # bind options (default = only localhost and tap, "all" for all interfaces)
      "is_profile" : true,  # whether we are using a connection profile - jump profile dir for config (default = false)

      # [Server Configuration]
      "sb" : {
        # the URL at which Switchboard should be contacted
        "url" : "https://switchboard-host/"
      },
      "setup" : {
        "enabled" : true    # whether setup phase with config download is required - e.g. Switchboard (default=true)
      },

      # [Logging Configuration]
      "log" : {
        # general verbosity level (0=low .. 7=high)
        "lev" : 3  
      },

      # [System Paths]
      # Note: readonly unless stated otherwise
      "paths" : {
        "sbinPrefix" : "/sbin",
        "confPrefix" : "/etc",
        "dataPrefix" : "/var/lib",
        "logPrefix" : "/var/log",
        "userDataPrefix" : "/home/myuser"       # read-write
      },

      # [VPN Setttings]
      # Note that encloud logs are normally routed to the client via clientPort in Setup API [API.md].
      "vpn" : {
        "verb" : 7,                   # verbosity level (0=low .. 7=high, default = 3)
        "exe" : "myopenvpn",          # executable name
        "conf" : "myopenvpn.conf",    # OpenVPN config filename     
        "fallback_conf" : "openvpn-fallback.conf"   # fallback OpenVPN config filename
      },

      # [SSL Setttings] 
      # Normally no intervention is required here
      "ssl" : {
        "init" : {                              # initialization channel overrides
          "sb" : {
            "url" : "https://initialization-host/"
          },
          "auth" : "x509",                      # "user-pass" (default), "x509"
          "verify_ca" : "false",                # "true" (default), "false"
          "ca" : "init_ca.pem",                 # REQUIRED for "auth" = "x509" / "cert-key"
          "cert" : "init_cert.pem",             # REQUIRED for "auth" = "x509" / "cert-key"
          "key" : "init_key.pem",               # REQUIRED for "auth" = "x509" / "cert-key"
          "p12" : "init.p12"                    # REQUIRED for "auth" = "x509" / "pkcs12"
        },
      
        "op" : {                                # operation channel overrides
          "sb" : {
            "url" : "https://operation-host/"   
          },
          "auth" : "x509",                      # "user-pass" (default), "x509"
          "verify_ca" : "false",                # "true" (default), "false"
          "ca" : "op_ca.pem",                   # REQUIRED for "auth" = "x509" / "cert-key"
          "cert" : "op_cert.pem",               # REQUIRED for "auth" = "x509" / "cert-key"
          "key" : "op_key.pem",                 # REQUIRED for "auth" = "x509" / "cert-key"
          "p12" : "op.p12"                      # REQUIRED for "auth" = "x509" / "pkcs12"
        }
      }

      # [ECE-only]
      "poi" : "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"    # Proof of Identity
      "csr" : {                                         # Certificate Request template
        "tmpl" : "csr-tmpl.json"              
      },
      "rsa" : {                                         # key size for generated RSA keys
        "bits" : 2048
      }
    }
