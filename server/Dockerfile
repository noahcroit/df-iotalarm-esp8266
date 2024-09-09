FROM python:3.10

COPY df_mqtt2socket.py config.json requirements.txt /home/iotworker/

WORKDIR /home/iotworker/

RUN apt-get update && apt-get upgrade
RUN python -m pip install -r requirements.txt

EXPOSE 8080
EXPOSE 1883
EXPOSE 1884
EXPOSE 1885

CMD ["python", "df_mqtt2socket.py", "-j", "config.json"]


