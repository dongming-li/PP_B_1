# THIS WEB APPLICATION UTILIZES THE DOCKER CONTAINER TECHNOLOGY

## PRODUCTION ENV: Deploying to DigitalOcean (DO)
You will need a Digital Ocean account in order to deploy to Digital Ocean (wow, shocker). Make an account, get some money or credit in there. Navigate to the API page and create a new Personal Access Token with both read and write permissions. Copy the token now.

Next, you need to have docker (engine), docker-compose, and docker-machine all installed. Google how to do this - the Docker documentation is very helpful with this. 

Go to the /frontend directory of the repo and run the following commands:
`docker-machine create --driver digitalocean --digitalocean-access-token={your_access_token} {name_of_droplet}`

`docker-machine env {name_of_droplet}`

`eval $(docker-machine env {name_of_droplet}`

`docker-compose -f docker-compose.prod.yml build`

`docker-compose -f docker-compose.prod.yml up -d`

Note on the above command: `-d` runs the containers in the background. Ie., you won't see any debug info in your terminal. If you wish to see this info, remove the `-d` but realize that as soon as you `CTRL-C` to exit the debug info, all the containers go down on the droplet. Think of `-d` as persistent.

Don't forget to create a superuser and collect static files before beginning real use of the webapp (see below for how to do this).

## DEV ENV: Deploying on your local machine
To deploy the webapp on your local machine you need to have docker (aka Docker Engine) and docker-compose installed. You do NOT need docker-machine but you CAN use it if desired. Docker has plenty of documentation on their site regarding docker-machine, but we will not be using it in the following example.

First, change your directory to /frontend in the repo. Next, run the following command to build the containers: `docker-compose -f docker-compose.yml build`

After the containers build, bring them up by typing `docker-compose -f docker-compose.yml up`

Technically `up` builds *AND* ups the containers but build will allow you to debug issues before the containers are ever brought up.

Don't forget to create a superuser and collect static files before beginning real use of the webapp (see below for how to do this).

## Note on static files:
NGINX is our reverse proxy and handles serving up static files. However, you must
tell Django to put the static files in the right place. To do this, run this
command whenever you alter/add static files (and after the first build!)...do this
only AFTER the docker containers are up and running: 
`docker exec -u 0 web python manage.py collectstatic --noinput`

This executes the command as root. There is no way to automate this because of permissions
issues with Docker. If you find a solution yourself, do share.

## The following commands are useful:
To build the image but not start the container:
`docker-compose build`

Builds image ONLY if it hasn't been built before, then starts the containers:
`docker-compose up`

Forces build or rebuild of containers and then starts them up in one command:
`docker-compose up --build`

You may append a `-d` to run the docker app in the background

See the RUNNING docker containers:
`docker ps`

See ALL docker containers:
`docker ps -a`

A lot of shit gets cached by Docker, here's some nice cleanup commands:
`docker rm $(docker ps -qa --no-trunc --filter "status=exited") && \
 docker system prune -a -f && \
 docker volume rm $(docker volume ls -qf dangling=true)`

## IMPORTANT: Set your own superuser
After everything here is up and running, there will be NO users and therefore no superusers that you can log into via /admin on the webapp. To create a superuser, run the following command in the Django shell: `python manage.py createsuperuser` ... you will be prompted for a username and password.

To get a Django shell after Docker containers are running, run the following command: `docker exec -it django bash`

## Explanation of this part of the repo:
django/ houses all files for the django app

"malweb" is the name of the Django project
/malweb is our Django python environment

"implantapp" is the name of the app (webapp) that our clients use to control their implants
/implantapp is our webapp directory

## More questions?
### Ask Daniel.
