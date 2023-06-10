import random
import smtplib
from email.mime.text import MIMEText
import sys

class sendAuthCode:
    def __init__(self,email):
        my_sender = '15265721868@163.com'
        my_pass = 'IJKJXGZGSGVAQVND'
        global email_code
        email_code = self.randomCode()
        try:
            msg = '<!DOCTYPE html><html><head><title>text</title></head><body><p style="font-size: 48px">' + email_code + '</p></body></html>'
            msgText = MIMEText(msg, 'html', 'utf-8')
            msgText['From'] = my_sender
            msgText['To'] = '2233945951@qq.com'
            msgText['Subject'] = "验证码"
            server = smtplib.SMTP_SSL("smtp.163.com", 465)
            server.login(my_sender, my_pass)
            server.sendmail(my_sender, email, msgText.as_string())
            with open("temp","w") as f:
                f.write(email_code);
        except Exception:
            print("验证码发送失败")

    def randomCode(self):
        return str(random.randint(100000, 999999))
def call():
    email=""
    with open("temp","r") as f:
        email=f.readline()
    code=sendAuthCode(email)


