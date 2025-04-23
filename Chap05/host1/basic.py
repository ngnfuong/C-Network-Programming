import socket
print("creating socket")
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("connecting to server...")
dhost = "google.com"
dport = 80
res = s.connect_ex((dhost, dport))

if res == 0:
    print(f"Connected to server {dhost} at port {dport}.")

else:
    print(f"Connection to server {dhost} at port {dport} failed.")
s.close()