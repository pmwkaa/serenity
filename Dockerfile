##
# NAME             : fgribreau/serenity
# VERSION          : latest
# DOCKER-VERSION   : 1.5
# DESCRIPTION      :
# TO_BUILD         : docker build --rm --pull=true --no-cache -t fgribreau/serenity .
# TO_SHIP          : docker push fgribreau/serenity
# TO_RUN           : docker run --rm -it -p 6379:6379 fgribreau/serenity
##

FROM debian:wheezy
MAINTAINER François-Guillaume Ribreau <docker@fgribreau.com>


COPY . /app

WORKDIR /app

RUN apt-get update && apt-get install -y build-essential git && make && sed -i "s/# bind 127.0.0.1/bind 0.0.0.0/g" serenity.conf

EXPOSE 6379
CMD ["./serenity", "serenity.conf"]
