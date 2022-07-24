# ESP32 Google Cloud IoT Core example

## Cloud IoT Core setup
1. Create a new GCP project or select an existing one
2. Enable the Cloud IoT Core API
3. Create your first Cloud Pub/Sub topic
``` bash
$ gcloud pubsub topics create my-topic
```
4. Create a subscription to the device's topic
``` bash
$ gcloud pubsub subscriptions create \
    projects/iot-core-tutorial-357222/subscriptions/my-subscription \
    --topic=my-topic
```
5. Clone the Cloud IoT Core Node.js sample files from GitHub
6. Create a device registry
``` bash
$ gcloud iot registries create \
    my-registry \
    --project=iot-core-tutorial-357222 \
    --region=us-central1 \
    --event-notification-config=topic=projects/iot-core-tutorial-357222/topics/my-topic
```
7. Create a device and add it to the registry
``` bash
$ gcloud iot devices create \
    my-node-device \
    --project=iot-core-tutorial-357222 \
    --region=us-central1 \
    --registry=my-registry \
    --public-key \
    path=rsa_cert.pem,type=rs256
```
8. Generate an Eliptic Curve (EC) private / public key pair:
``` bash
$ openssl ecparam -genkey -name prime256v1 -noout -out ec_private.pem
$ openssl ec -in ec_private.pem -pubout -out ec_public.pem
```
9. You will also need to extract your private key using the following command:
``` bash
$ openssl ec -in ec_private.pem -noout -text
```
10. Get root CA certificate
``` bash
$ openssl s_client -showcerts -connect mqtt.2030.ltsapis.goog:8883
```
