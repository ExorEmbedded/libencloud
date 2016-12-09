Encloud API v1
==============

### Definitions

     QCC        Branded Connect App
     ECE        Cloud Enabler device
     SECE       Software Cloud Enabler device

### Types

     <bool>     Boolean - "true" | "false"
     <short>    Integer between 0 and 65535
     <uuid>     Universally Unique Identifier - e.g. "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"
     <sb_url>   Switchboard URL
     <e_url>    Encloud URL

#### API Definition

##### State Retrieval

__GET <e_url>/api/v1/status__

Definitions:

    "state"

    -1      None
    0      Idle            (grey)
    1      Error           (red)
    2      Setting Up      (yellow)
    3      Connecting      (orange)
    4      Connected       (green)

e.g:

        '{
            "state" : 1,                    # string representation of <state>
            "error" : {                     # OPTIONAL: defined if state == 1 (StateError)
                "code" : 123,               # error code (Please refer to <encloud/Error> for definition)
                "seq" : 111,                # sequence number used to detect whether error instance has 
                                            # changed compared to previously retrieved value
                "desc" : "option string description of error",
                "extra" : "extra error message"
            },
            "progress" {                    # OPTIONAL
                "desc: "Step description",  # string representation of step
                "step": 2,                  # current step number 
                "step": 5                   # total number of steps
            },
            "fallback" : true,              # OPTIONAL (default=false) true if fallback VPN server is being used
            "need" : {                      # OPTIONAL needs to be fulfilled via API. Possible values:
                                            # "license" (SECE), "sb_auth", "proxy_auth"
                "license" : {},
                "sb_auth" : {
                    "domains" : [ "myorg1", "myorg2" ]     # optional domains (organizations)
                }
            }
        }'


##### Authentication/Login API 

__POST <e_url>/api/v1/auth__

Definitions:

    "id"            OPTIONAL authentication identifier: "sb" (default), "proxy"
    "type"          OPTIONAL type of authentication: "user-pass" (default), "x509" or "x509-user-pass" (2-factor)
    "url"           destination url (for Switchboard login or proxy)
    "user"          OPTIONAL user for authentication (type="user-pass" or "x509-user-pass")
    "pass"          OPTIONAL password for authentication (type="user-pass" or "x509-user-pass")
    "path"          OPTIONAL path (for type="x509")
    "p12pass"       OPTIONAL path (for type="x509-user-pass")

e.g:
    
    id=proxy&type=http&url=<url>&user=myuser&pass=mypass
    id=sb&type=user-pass&url=<url>&user=myuser&pass=mypass
    id=sb&type=x509&url=<url>&pass=mypass&path=c:\key.p12

##### Setup API

__GET <e_url>/api/v1/setup__ (ECE-only)

Definitions:

    "poi"           <uuid> proof of identity returned by ECE for Switchboard association

e.g:

    '{
        "poi" : "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"
     }'

__POST <e_url>/api/v1/setup__ (SECE-only)

Definitions:

    "license"       <uuid> license entered by user to activate SECE

e.g:                

    license=aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee

__POST <e_url>/api/v1/setup__ (QCC-only)

Definitions:

    "clientPort"    <short> Port at which client can be reached - set by GUI and used by Encloud 
                        to forward application open/close requests via Cloud API

e.g.                

    clientPort=12345

Definitions

    "logPort"       <short> Port at which LogListener can be reached - set by GUI and used by Encloud 

e.g.                

    logPort=12345

__GET <e_url>/api/v1/setup__ (QCC-only)

e.g
    
    '{
        "time" : 1396014836.523807,        # When the config data was received
        "server" : {
            "uuid" : "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee",
            "openvpn_internal_ip" : "192.168.150.1",
            "fallback_openvpn_internal_ip" : "192.168.151.1"
            "available_pages" : { "settings" : "/manage/access/settings/standalone/",      # [...]
    }'


##### Cloud API

__POST <e_url>/api/v1/cloud__

Definitions 

    "action"        Operation:                  Clients:

                    "start","stop"              GUI/browser
                    "syncRoutes"                Switchboard
                    "open","close"              Switchboard
                    "setGateway"                OpenVPN up script (Windows-only)

client -> encloud:

e.g: 

    action=start
    action=syncRoutes&ips=192.168.1.100,192.168.1.101
    action=setGateway&ip=192.168.151.1

encloud -> gui (QCC):

e.g: 
    
    action=open&name=myApp&endpoint=myEndpoint&remark=myRemark&path=
                    /%HOME_PATH%/to/File&args=param1%20param2
    action=close&name=myApp&endpoint=myEndpoint

Notes: 

    ID = name+endpoint


##### Config API

__GET <e_url>/api/v1/config__

    e.g.    '{
                 "log" : { "lev" : 3 }
            '}


__POST <e_url>/api/v1/config__

    "Content-Type" : application/json

e.g: 

    1.  { "timeout" : "300" }

    2.  { "ssl" : { "verify_ca" : "false" } }

    3.  { "log" : { "lev" : "7" } }


##### Switchboard Setup Notification (TODO)

__POST <sb_url>/manage/commands/commands.access.cloud.setup__


e.g:

    port=1234


#### Error Handling

Possible HTTP Statuses return as a result of GET/POST:

     LIBENCLOUD_HTTP_STATUS_OK                = 200
     LIBENCLOUD_HTTP_STATUS_BADREQUEST        = 400
     LIBENCLOUD_HTTP_STATUS_NOTFOUND          = 404
     LIBENCLOUD_HTTP_STATUS_BADMETHOD         = 405
     LIBENCLOUD_HTTP_STATUS_INTERNALERROR     = 500

Application errors are available "State Retrieval API" above.


#### Other Notes

##### Requests

All POST requests have Content-Type `application/x-www-form-urlencoded`, unless
otherwise specified.

##### Responses

By default, the Content-type of responses is `application/json`:

e.g:    

    '{ ... }'

Optionally (if requests contains the "callback" parameter), responses will
contain json wrapped in a javascript function (JSONP handling to bypass
same-origin policy). In such case the returned Content-Type will be
`application/javascript`.

e.g:    

    'myJsonpCallback({ ... })'

        , where callback="myJsonpCallback"
