from django.db import models
from django.forms import ModelForm

class Upload(models.Model):
    userID = models.CharField(max_length=32)
    implantID = models.CharField(max_length=32)
    file_type=models.CharField(max_length=16)
    data = models.FileField(upload_to="files/")
    upload_date=models.DateTimeField(auto_now_add = True)
    def __str__(self):
        return self.data.name

class UploadForm(ModelForm):
    class Meta:
        model = Upload
        fields = ['data', 'userID', 'implantID', 'file_type']

class Implant(models.Model):
    implantID = models.CharField(max_length=32)
    userID = models.CharField(max_length=32)
    command = models.CharField(max_length=32)
    macAddress = models.CharField(max_length=20)
    ipAddress = models.CharField(max_length=16)
    def __str__(self):
        return self.ipAddress

class PartialRegisterForm(ModelForm):
    class Meta:
        model = Implant
        fields = ['implantID', 'userID', 'macAddress']
        exclude = ['ipAddress', 'command']
