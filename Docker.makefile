run-mosquitto:
	docker pull eclipse-mosquitto:latest
	docker run -d --name mosquitto -p 30520:1883\
	  -v "$(PWD)/docker/mosquitto/data:/mosquitto/data"\
	   -v "$(PWD)/docker/mosquitto/log:/mosquitto/log"\
	    eclipse-mosquitto:latest