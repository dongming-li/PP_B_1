FROM python:3.6
ENV PYTHONUNBUFFERED 1

ENV APP_USER djangouser
ENV APP_ROOT /django

# --home-dir takes care of creating /django so no need for mkdir
# NOTE: Change the number after -u (default 1000) to the UID of the user running the docker commands
# This way the container will run with privs that you have. Get UID by running echo $UID
RUN groupadd -r ${APP_USER} && \ 
    useradd -r -m --home-dir ${APP_ROOT} -g ${APP_USER} ${APP_USER} -u 1000 && \ 
    mkdir /var/www/ && \ 
    mkdir /var/www/static && \
    mkdir /config

COPY --chown=djangouser:djangouser /django /django

COPY --chown=djangouser:djangouser /config/requirements.txt /config/
RUN pip install -r /config/requirements.txt

WORKDIR ${APP_ROOT}
USER ${APP_USER}
