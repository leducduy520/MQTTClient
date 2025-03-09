run-mosquitto-image:
	docker run -d --name mosquitto -p 30520:1883\
	 -v "$PWD/docker/mosquitto/config:/mosquitto/config"\
	  -v "$PWD/docker/mosquitto/data:/mosquitto/data"\
	   -v "$PWD/docker/mosquitto/log:/mosquitto/log"\
	    eclipse-mosquitto:latest /usr/sbin/mosquitto -c /mosquitto/config/mosquitto.conf -v