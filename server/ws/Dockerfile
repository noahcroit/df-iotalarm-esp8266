FROM python:3.10

COPY wsserver.py mqtt_worker.py config.json requirements.txt /home/iotworker/

WORKDIR /home/iotworker/

RUN apt-get update && apt-get upgrade -y
RUN python -m pip install -r requirements.txt

EXPOSE 8080
EXPOSE 1883
EXPOSE 1884
EXPOSE 1885
EXPOSE 6379

CMD ["python", "wsserver.py", "-j", "config.json"]


