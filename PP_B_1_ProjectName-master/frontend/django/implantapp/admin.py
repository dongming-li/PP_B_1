from django.contrib import admin

from .models import Implant
from .models import Upload

admin.site.register(Implant)
admin.site.register(Upload)
