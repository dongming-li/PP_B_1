"""malweb URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/1.11/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  url(r'^$', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  url(r'^$', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.conf.urls import url, include
    2. Add a URL to urlpatterns:  url(r'^blog/', include('blog.urls'))
"""
from django.conf.urls import include, url
from django.contrib import admin
from django.contrib.auth import views as auth_views
from django.conf import settings
from django.views.static import serve
from implantapp import views

urlpatterns = [
  url(r'^heartbeat/', views.heartbeat, name='heartbeat'),
  url(r'^register_implant/', views.register, name='registerImplant'),
  url(r'^upload/', views.upload, name='fileupload'),
  url(r'^account/', include('django.contrib.auth.urls')),
  url(r'^signup/', views.signup, name='signup'),
  url(r'^admin/', admin.site.urls),
  url(r'^implants/', views.implants, name='implants'),
  url(r'^assets/', views.assets, name='assets'),
  url(r'^help/', views.help, name='help'),
  url(r'^create/', views.create, name='create'),
  url(r'^contact/', views.contact, name='contact'),
  url(r'^$', views.index),
]

# apparently django can't do directory listings?
# TODO see if we can get this to work if we continue developing this app
#if settings.DEBUG is True:
#    urlpatterns += [
#        url(r'^media/(?P<path>.*)$', serve, {
#            'document_root': settings.MEDIA_ROOT,
#        }),
#    ]
