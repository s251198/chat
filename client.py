import tkinter as tk
from tkinter import scrolledtext
import socket
import threading

class ChatClient:
    def __init__(self, root):
        self.root = root
        self.root.title("Chat Client")

        self.server_ip = tk.StringVar()
        self.server_port = tk.IntVar(value=8080)  # Default port 8080
        self.message = tk.StringVar()

        self.connected = False
        self.client_socket = None

        self.create_widgets()

    def create_widgets(self):
        tk.Label(self.root, text="Server IP:").pack()
        tk.Entry(self.root, textvariable=self.server_ip).pack()
        
        tk.Label(self.root, text="Server Port:").pack()
        tk.Entry(self.root, textvariable=self.server_port).pack()
        
        self.connect_button = tk.Button(self.root, text="Connect", command=self.connect_to_server)
        self.connect_button.pack()

        self.chat_area = scrolledtext.ScrolledText(self.root, state='disabled', width=50, height=20)
        self.chat_area.pack()

        tk.Label(self.root, text="Message:").pack()
        tk.Entry(self.root, textvariable=self.message).pack()

        self.send_button = tk.Button(self.root, text="Send", command=self.send_message)
        self.send_button.pack()

        self.disconnect_button = tk.Button(self.root, text="Disconnect", command=self.disconnect, state='disabled')
        self.disconnect_button.pack()

    def connect_to_server(self):
        ip = self.server_ip.get()
        port = self.server_port.get()

        try:
            self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.client_socket.connect((ip, port))
            self.connected = True
            self.connect_button.config(state='disabled')
            self.disconnect_button.config(state='normal')
            self.receive_thread = threading.Thread(target=self.receive_messages)
            self.receive_thread.start()
        except Exception as e:
            self.display_message(f"Connection error: {e}")

    def receive_messages(self):
        while self.connected:
            try:
                message = self.client_socket.recv(1024).decode()
                if message:
                    self.display_message(message)
                else:
                    self.connected = False
                    self.display_message("Server closed connection.")
            except:
                self.connected = False
                self.display_message("Connection lost.")
                break
        self.disconnect()

    def display_message(self, message):
        self.chat_area.config(state='normal')
        self.chat_area.insert(tk.END, message + '\n')
        self.chat_area.config(state='disabled')
        self.chat_area.yview(tk.END)

    def send_message(self):
        if self.connected:
            message = self.message.get()
            self.client_socket.send(message.encode())
            self.message.set("")

    def disconnect(self):
        if self.connected:
            self.client_socket.close()
            self.connected = False
        self.connect_button.config(state='normal')
        self.disconnect_button.config(state='disabled')
        self.display_message("Disconnected from server.")

if __name__ == "__main__":
    root = tk.Tk()
    client = ChatClient(root)
    root.mainloop()
