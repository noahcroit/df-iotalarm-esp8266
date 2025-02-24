import sys
import asyncio
import async_timeout
import aioredis
import aiomqtt
import requests
import queue
import argparse
import json
import logging



async def task_mqttsub():
    global cfg
    global q2client
    logger.info('Starting a MQTT subscribe task for receiving device info')
    while True:
        try:
            async with aiomqtt.Client(cfg['mqtt_broker'], int(cfg['mqtt_port'])) as client:
                await client.subscribe(cfg['mqtt_topics'][0])
                await client.subscribe(cfg['mqtt_topics'][1])
                await client.subscribe(cfg['mqtt_topics'][2])
                async for message in client.messages:
                    payload = message.payload.decode('utf-8')
                    data = {"topic": str(message.topic), "message":payload}
                    data_json = json.dumps(data)
                    q2client.put(data_json)
                    logger.info('data from device : %s', data_json)
        except Exception as e:
            logger.warning("task: mqtt subscribe will be stop")
            break
        await asyncio.sleep(1)



async def task_mqttpub():
    global cfg
    global q2device
    logger.info('Starting a MQTT publish task for sending OTA to device')
    while True:
        try:
            if not q2device.empty():
                l_device_id=[]
                data = q2device.get().decode('utf-8')
                data = json.loads(data)
                if data['topic'] == 'iotalarm/OTA':
                    if data['ID'] == 'ALL':
                        logger.warning("ota request ALL from frontend")
                        l_device_id = get_device_list()
                    else:
                        logger.warning("ota request from frontend : %s", data['ID'])
                        l_device_id.append(str(data['ID']))

                    for device_id in l_device_id:
                        async with aiomqtt.Client(cfg['mqtt_broker'], int(cfg['mqtt_port'])) as client:
                            await client.publish(cfg['mqtt_topics'][3], payload=device_id)
                            logger.info("ota topic : %s, payload=%s", cfg['mqtt_topics'][3], device_id)
                        await asyncio.sleep(1)
        except Exception as e:
            logger.warning("task: mqtt publish will be stop")
            break
        await asyncio.sleep(1)



async def task_redissub(channel: aioredis.client.PubSub):
    global q2device
    while True:
        try:
            async with async_timeout.timeout(1):
                message = await channel.get_message(ignore_subscribe_messages=True)
                if message is not None:
                    q2device.put(message['data'])
                await asyncio.sleep(0.1)
        except asyncio.TimeoutError:
            pass



async def task_redispub(redis, channel_pub):
    global q2client
    while True:
        if not q2client.empty():
            txdata = q2client.get()
            await redis.publish(channel_pub, txdata)
        await asyncio.sleep(1)



def get_device_list():
    global cfg
    logger.info('Requesting the list of all devices ID from django server')
    r = requests.get(cfg['API_request_id'])
    if r.status_code == 200:
        logger.info('Request OK')
        response_text = r.text
        response_text = response_text[1:-1]
        l = response_text.split(',')
        return l
    else:
        logger.error('Request Failed')
        return None



async def main():
    global cfg
    # Initialize parser
    parser = argparse.ArgumentParser()
    # Adding optional argument
    # Read arguments from command line
    parser.add_argument("-j", "--json", help="JSON file for the configuration", default='config.json')
    args = parser.parse_args()

    # Extract config data from .json
    try:
        f = open(args.json, 'rb')
        cfg = json.load(f)
        f.close()
    except OSError:
        logger.error('Configuration file does not exist!')
        sys.exit()

    # REDIS initialization
    logger.info('REDIS client initialize')
    redis = aioredis.from_url("redis://localhost")
    pubsub = redis.pubsub()
    await pubsub.subscribe('ch:from_client')
    logger.info('REDIS OK')

    # create task(s)
    t_rsub = asyncio.create_task(task_redissub(pubsub))
    t_rpub = asyncio.create_task(task_redispub(redis, 'ch:from_device'))
    t_mqttsub = asyncio.create_task(task_mqttsub())
    t_mqttpub = asyncio.create_task(task_mqttpub())

    # main loop
    restart_cnt=0
    while True:
        logger.info('Checking the status of all tasks')
        await asyncio.sleep(1)
        # check tasks
        if t_rsub.done():
            logger.info('Restart a task_redissub')
            t_rsub = asyncio.create_task(task_redissub(pubsub))
        if t_rpub.done():
            logger.info('Restart a task_redispub')
            t_rpub = asyncio.create_task(task_redispub(redis, 'ch:from_device'))
        if t_mqttsub.done():
            logger.info('Restart a task_mqttsub')
            t_mqttsub = asyncio.create_task(task_mqttsub())
        if t_mqttpub.done():
            logger.info('Restart a task_mqttpub')
            t_mqttpub = asyncio.create_task(task_mqttpub())

        # restart schedule for every 6 hours
        if restart_cnt >= 21600:
            restart_cnt=0
            t_rsub.cancel()
            t_rpub.cancel()
            t_mqttsub.cancel()
            t_mqttpub.cancel()
            t_rsub = asyncio.create_task(task_redissub(pubsub))
            t_rpub = asyncio.create_task(task_redispub(redis, 'ch:from_device'))
            t_mqttsub = asyncio.create_task(task_mqttsub())
            t_mqttpub = asyncio.create_task(task_mqttpub())
        restart_cnt += 1



if __name__ == '__main__':
    cfg = None
    q2client = queue.Queue(maxsize=256)
    q2device = queue.Queue(maxsize=256)

    # setup logging system
    logger = logging.getLogger(__name__)
    logger.setLevel(logging.DEBUG)
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.INFO)
    console_handler.setFormatter(formatter)
    logger.addHandler(console_handler)

    # run main program
    logger.info('Starting an IoT alarm MQTT worker')
    asyncio.run(main())
