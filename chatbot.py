import tkinter as tk

# Chatbot response function
def respond():
    user_input = entry.get().lower()
    response = ""

    if user_input == "exit":
        root.destroy()

    elif user_input in ["hello", "hi", "hey"]:
        response = "Hello! How can I help you today?"

    elif user_input == "what is ai?" or user_input == "what is artificial intelligence?":
        response = "AI stands for Artificial Intelligence. It's the simulation of human intelligence by machines."

    elif user_input == "how are you?":
        response = "I'm just a bunch of code, but I'm functioning as expected!"

    elif user_input == "tell me a joke":
        response = "What do you call a tiger that's sleeping? One word — tigurrrr!"

    else:
        response = "Sorry, I don't understand that."

    chat_log.insert(tk.END, "You: " + user_input + "\n")
    chat_log.insert(tk.END, "Chatbot: " + response + "\n\n")
    entry.delete(0, tk.END)

# Create GUI window
root = tk.Tk()
root.title("Simple Chatbot")
root.geometry("400x500")

# Chat history text area
chat_log = tk.Text(root, bg="white", fg="black", font=("Arial", 12))
chat_log.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

# Input field
entry = tk.Entry(root, font=("Arial", 14))
entry.pack(padx=10, pady=(0, 10), fill=tk.X)
entry.bind("<Return>", lambda event: respond())

# Send button
send_button = tk.Button(root, text="Send", command=respond)
send_button.pack(pady=(0, 10))

# Start GUI loop
root.mainloop()
