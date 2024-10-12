MQTT-to-Websocket (mqtt2ws) worker

To build the image
``` 
docker build -t df-iotalarmworker .
```

Run mqtt2ws worker's container with built image
```
docker run --network host -d --name iotalarm-socket2mqtt-worker df-iotalarmworker
```
