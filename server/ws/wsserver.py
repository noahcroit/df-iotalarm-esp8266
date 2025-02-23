import sys
import asyncio
import async_timeout
import websockets
import aioredis
import queue
import argparse
import json
import logging



async def connection_handler(websocket):
    global num_client
    global seq
    global q2device

    # count new connection
    client_id = seq
    num_client += 1
    seq += 1
    logger.info('New client connected. Current num=%d', num_client)

    # create new task for sending data to client of this connection
    t_tx = asyncio.create_task(task_send2client(websocket, client_id))

    # loop for receiving data from client
    while True:
        try:
            request = await websocket.recv()
            q2device.put(request)
        except websockets.ConnectionClosedOK:
            num_client -= 1
            logger.info('Client disconnected. Current num=%d', num_client)
            t_tx.cancel()
            break
        await asyncio.sleep(0.1)



async def task_send2client(websocket, client_id):
    global q2client
    while True:
        if not q2client.empty():
            logger.info('Sending a device info to client ID=%d', client_id)
            txdata = q2client.get()
            await websocket.send(txdata)
        await asyncio.sleep(0.1)



async def task_run_wsserver():
    global cfg
    logger.info('Starting a websocket server task')
    from websockets.asyncio.server import serve
    async with serve(connection_handler, "", cfg['ws_port']):
        await asyncio.get_running_loop().create_future()



async def task_redissub(channel: aioredis.client.PubSub):
    global q2client
    logger.info('Starting a REDIS subscribe task for receiving device info')
    while True:
        try:
            async with async_timeout.timeout(1):
                message = await channel.get_message(ignore_subscribe_messages=True)
                if message is not None:
                    txdata = message["data"].decode()
                    logger.info('REDIS Message received: %s', txdata)
                    for n in range(num_client):
                        if q2client.full():
                            discard = q2client.get()
                        q2client.put(txdata)
                await asyncio.sleep(0.1)
        except asyncio.TimeoutError:
            pass



async def task_redispub(redis, channel_pub):
    global q2device
    logger.info('Starting a REDIS publish task for sending OTA request')
    while True:
        if not q2device.empty():
            rxdata = q2device.get()
            logger.info('REDIS Publish OTA request to device(s)')
            await redis.publish(channel_pub, str(rxdata))
        await asyncio.sleep(1)



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
    await pubsub.subscribe('ch:from_device')
    logger.info('REDIS OK')

    # create task(s)
    t_rsub = asyncio.create_task(task_redissub(pubsub))
    t_rpub = asyncio.create_task(task_redispub(redis, 'ch:from_client'))
    t_ws = asyncio.create_task(task_run_wsserver())

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
            t_rpub = asyncio.create_task(task_redispub(redis, 'ch:from_client'))
        if t_ws.done():
            logger.info('Restart a task_run_wsserver')
            t_ws = asyncio.create_task(task_run_wsserver())

        # restart schedule for every 6 hours
        if restart_cnt >= 21600:
            restart_cnt=0
            t_rsub.cancel()
            t_rpub.cancel()
            t_ws.cancel()
            t_rsub = asyncio.create_task(task_redissub(pubsub))
            t_rpub = asyncio.create_task(task_redispub(redis, 'ch:from_client'))
            t_ws = asyncio.create_task(task_run_wsserver())
        restart_cnt += 1



if __name__ == '__main__':
    # setup global vars.
    cfg = None
    num_client=0
    seq=0
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
    logger.info('Starting an IoT alarm websocket server')
    asyncio.run(main())
