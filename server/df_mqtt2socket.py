import asyncio
import async_timeout
import aiomqtt
import websockets
import queue
import time
import logging
import argparse
import json



cfg = None
q_ws2mqtt = queue.Queue()
q_mqtt2ws = queue.Queue()
isrun_mqttsub=False
isrun_mqttpub=False
isrun_wsserver=False



async def task_mqttsub():
    global cfg
    global isrun_mqttsub
    global q_mqtt2ws
    isrun_mqttsub=True
    while isrun_mqttsub:
        try:
            async with aiomqtt.Client(cfg['mqtt_broker'], int(cfg['mqtt_port'])) as client:
                await client.subscribe(cfg['mqtt_topics'][0])
                await client.subscribe(cfg['mqtt_topics'][1])
                await client.subscribe(cfg['mqtt_topics'][2])
                async for message in client.messages:
                    payload = message.payload.decode('utf-8')
                    data = {"topic": str(message.topic), "message":payload}
                    data_json = json.dumps(data)
                    q_mqtt2ws.put(data_json)
                    logging.warning('data from device : %s', data_json)
        except Exception as e:
            #logging.error(e)
            logging.warning("task mqttsub will be stop")
            isrun_mqttsub=False
        await asyncio.sleep(1)

async def task_mqttpub():
    global cfg
    global isrun_mqttpub
    global q_ws2mqtt
    isrun_mqttpub=True
    while isrun_mqttpub:
        try:
            if not q_ws2mqtt.empty():
                data = q_ws2mqtt.get()
                if data['topic'] == 'iotalarm/OTA':
                    if data['ID'] == 'ALL':
                        upgrade_firmware_all()
                    else:
                        logging.warning("ota request from frontend : %s", data['ID'])
                        device_id = data['ID']
                    async with aiomqtt.Client(cfg['mqtt_broker'], int(cfg['mqtt_port'])) as client:
                        await client.publish(cfg['mqtt_topics'][3], payload=str(device_id))
                        logging.warning("ota topic : %s, payload=%s", cfg['mqtt_topics'][3], data['ID'])
        except Exception as e:
            #logging.error(e)
            logging.warning("task mqttpub will be stop")
            isrun_mqttpub=False
        await asyncio.sleep(1)



async def ws_handler(websocket):
    global isrun_wsserver
    t_send2wsclient = asyncio.create_task(task_send2wsclient(websocket))
    while isrun_wsserver:
        try:
            # receiving ws
            rxdata = await websocket.recv()
            logging.warning("recv data from frontend = %s", rxdata)
            data = json.loads(rxdata)
            q_ws2mqtt.put(data)
        except websockets.ConnectionClosedOK:
            t_send2wsclient.cancel()
            break
        await asyncio.sleep(1)

async def task_send2wsclient(websocket):
    global q_mqtt2ws
    global isrun_wsserver
    while isrun_wsserver:
        try:
            # sending ws
            if not q_mqtt2ws.empty():
                txdata = q_mqtt2ws.get()
                logging.warning("send data to frontend %s", txdata)
                await websocket.send(txdata)
        except websockets.ConnectionClosedOK:
            break
        await asyncio.sleep(1)

async def task_wsserver():
    global cfg
    global isrun_wsserver
    global q_mqtt2ws
    isrun_wsserver=True
    from websockets.asyncio.server import serve
    async with serve(ws_handler, "", int(cfg['ws_port'])):
        await asyncio.get_running_loop().create_future()
    isrun_wsserver=False
    logging.warning("task websocket server will be stop")



async def upgrade_firmware_all():
    pass



async def main():
    global cfg

    # Initialize parser
    parser = argparse.ArgumentParser()
    # Adding optional argument
    # Read arguments from command line
    parser.add_argument("-j", "--json", help="JSON file for the configuration", default='config.json')
    args = parser.parse_args()

    # Extract config data from .json
    f = open(args.json)
    cfg = json.load(f)
    f.close()

    # Create async tasks
    asyncio.create_task(task_mqttsub())
    asyncio.create_task(task_mqttpub())
    asyncio.create_task(task_wsserver())

    while True:
        logging.warning("checking task(s) status...")
        await asyncio.sleep(5)

        # check tasks
        if not isrun_mqttsub:
            asyncio.create_task(task_mqttsub())
        if not isrun_mqttpub:
            asyncio.create_task(task_mqttpub())
        if not isrun_wsserver:
            asyncio.create_task(task_wsserver())




if __name__ == '__main__':
    asyncio.run(main())
