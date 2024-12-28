# Correct

# 2 different clients form differnet ip/port operating concurrently 

import threading
import subprocess
import time

def run_client_operations(port, operations, log_file):
    with open(log_file, "w") as log:
        process = subprocess.Popen(["../../../client/build/bin/client", "127.0.0.1", str(port)],
                                    stdin=subprocess.PIPE,
                                    stdout=log,
                                    stderr=log,
                                    text=True)
        try:
            for operation in operations:
                process.stdin.write(f"{operation}\n")
                process.stdin.flush()
                # time.sleep(1)  # Delay to simulate user input
            process.stdin.write("8\n")  # Exit
            process.stdin.flush()
            process.communicate()  # Wait for the process to finish
        except Exception as e:
            log.write(f"Error in client on port {port}: {e}\n")
        finally:
            process.terminate()

client1_operations = [
    "2",              # Login
    "sidsg",          # Username
    "Sidsg1234",      # Password
    "3",              # List files
    "4",              # Upload file
    "/home/sid/Pictures/spike.png",   # File to upload
    "3",              # List files
    "6",              # Rename file
    "spike.png",      # Old filename
    "spike2.png",     # New filename
    "7",              # Delete file
    "spike.png",      # File to delete
    "3",              # List files
    "5",              # Download file
    "spike.png",      # File to download
    "./",             # Destination path
    "5",              # Download file
    "spike2.png",     # File to download
    "./",             # Destination path
    "7",              # Delete file
    "spike2.png"      # File to delete
]

client2_operations = [
    "2",              # Login
    "mkbhd",          # Username
    "Mkbhd1234",      # Password
    "3",              # List files
    "4",              # Upload file
    "/home/sid/Pictures/spike.png",   # File to upload
    "3",              # List files
    "6",              # Rename file
    "spike.png",      # Old filename
    "spike3.png",     # New filename
    "7",              # Delete file
    "spike.png",      # File to delete
    "3",              # List files
    "5",              # Download file
    "spike.png",      # File to download
    "./",             # Destination path
    "5",              # Download file
    "spike3.png",     # File to download
    "./",             # Destination path
    "7",              # Delete file
    "spike3.png"      # File to delete
]

thread1 = threading.Thread(target=run_client_operations, args=(7000, client1_operations, "client1.log"))
thread2 = threading.Thread(target=run_client_operations, args=(7000, client2_operations, "client2.log"))

thread1.start()
thread2.start()

thread1.join()
thread2.join()

print("Test completed.")
