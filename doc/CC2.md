
#### Compilation flags

    $ qmake -r CONFIG+="exor splitdeps modeqcc noservice"
    

### Runtime usage

Given the following variable definitions (and customizing them where necessary):

    $ CLOUD_URL="https://pairing.corvina.io/api/v1/"
    $ CLOUD_ACTIVATION_CODE='75BE-4671-99D9-3CDA'
    $ SERVICE_URL="http://localhost:4804/api/v1"


#### 1. Configuration

    {
        ...
        "setup" : {
             "enabled" : true,
             "cc2_agent" : true
        }
        ...
    }        


#### 2. Authentication

Then setup authentication data:

    $ curl "${SERVICE_URL}/auth" -d "url=${CLOUD_URL}" -d "type=activation-code" -d "activation-code=${CLOUD_ACTIVATION_CODE}" 
    

# example
curl http://localhost:4804/api/v1/auth -d "type=activation-code" -d "pass=75BE-4671-99D9-3CDA" -d "id=sb" -d "url=https://pairing.corvina.io/api/v1/" && curl http://localhost:4804/api/v1/cloud -d "action=start"



