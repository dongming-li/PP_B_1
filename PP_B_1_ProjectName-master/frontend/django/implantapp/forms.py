from django import forms

class UserRegistrationForm(forms.Form):
  username = forms.CharField(
    required = True,
    label = 'Username',
    max_length = 32
  )
  password = forms.CharField(
    required = True,
    label = 'Password',
    max_length = 32,
    widget = forms.PasswordInput()
  )
  password_confirm = forms.CharField(
    required = True,
    label = 'Confirm Password',
    max_length = 32,
    widget = forms.PasswordInput()
  )
  invite_code = forms.CharField(
    required = True,
    label = 'Invite Code',
    max_length = 32,
    widget = forms.PasswordInput()
  )

class HeartbeatForm(forms.Form):
  userID = forms.CharField(label='userID', max_length=32)
  botID = forms.CharField(label='botID', max_length=32)

class CommandForm(forms.Form):
  implantID = forms.CharField(label='implantID', max_length=32)
  command = forms.CharField(label='command', max_length=2) 
