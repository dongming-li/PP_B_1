#!/bin/bash
if which docker >/dev/null && which docker-compose >/dev/null; then
  echo Deploying the dockerized webapp in LOCAL, DEVELOPMENT ENVIRONMENT.
  docker-compose -f docker-compose.yml build && \
  docker-compose -f docker-compose.yml up -d
  docker exec -u 0 web python manage.py collectstatic --noinput
  echo Entering a tty bash shell. Enter the following command and follow the prompts to set up a Django Superuser:
  echo BEGIN COPY COMMAND ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  echo python manage.py createsuperuser
  echo END COPY COMMAND ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  echo After you create the superuser simply type exit to leave the shell in the web container.
  docker exec -it web bash 
else
  echo ERROR: Make sure that docker AND docker-compose are both installed.
fi
