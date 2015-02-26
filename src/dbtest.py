#!/usr/bin/python
# -*- coding: utf-8 -*-
from peewee import *
import datetime

db = SqliteDatabase('my_database.db', threadlocals=True)

class BaseModel(Model):
    class Meta:
        database = db

class User(BaseModel):
    username = CharField(unique=True)

class Tweet(BaseModel):
    user = ForeignKeyField(User, related_name='tweets')
    message = TextField()
    created_date = DateTimeField(default=datetime.datetime.now)
    is_published = BooleanField(default=True)

db.connect()
db.create_tables([User, Tweet], safe=True)


charlie = User.create(username='charlie')
huey = User(username='huey')
huey.save()

# No need to set `is_published` or `created_date` since they
# will just use the default values we specified.
Tweet.create(user=charlie, message='My first tweet')
Tweet.create(user=charlie, message='My second tweet')

# A simple query selecting a user.
ret = User.get(User.username == 'charlie')
print ret
for t in ret.tweets:
	print t.message
# Get tweets created by one of several users. The "<<" operator
# corresponds to the SQL "IN" operator.
usernames = ['charlie', 'huey', 'mickey']
users = User.select().where(User.username << usernames)
tweets = Tweet.select().where(Tweet.user << users)
