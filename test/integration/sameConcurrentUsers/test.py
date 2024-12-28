# synchromnization bug present

"""
concurrent users with same username login from different ports/ip

client1: login, username, password
client2: login, username, password
client1: upload the file /home/sid/Pictures/spike.png first then 
client2: list files
client1: list files
client1: rename req
client2: delete req
client1: provide filename to rename
client2: provide filename to delete and deletes the file
client1: provide new filename to which it has to be renamed

finally client2 delete operation is success and client 1 rename operation is failure
"""


import threading
import subprocess
import time

def run_client_operations(client_id, port, operations, log_file, condition, step):
    with open(log_file, 'w') as log:
        process = subprocess.Popen(
            ["../../../client/build/bin/client", "127.0.0.1", str(port)],
            stdin=subprocess.PIPE,
            stdout=log,
            stderr=log,
            text=True
        )
        try:
            for operation in operations:
                if isinstance(operation, tuple):
                    op, required_step, next_step = operation
                    with condition:
                        while step[0] != required_step:
                            print(f"[{client_id}] Waiting for step {required_step}")
                            success = condition.wait(timeout=10)
                            if not success:
                                print(f"[{client_id}] Timeout waiting for step {required_step}")
                                return
                        print(f"[{client_id}] Executing operation in step {required_step}")
                        if op is not None:
                            process.stdin.write(f"{op}\n")
                            process.stdin.flush()
                        time.sleep(1)
                        step[0] = next_step
                        condition.notify_all()
                else:
                    process.stdin.write(f"{operation}\n")
                    process.stdin.flush()
                    time.sleep(1)
            process.stdin.write("8\n")  # Exit
            process.stdin.flush()
            process.wait()  # Wait for proper client shutdown
        except Exception as e:
            print(f"[{client_id}] Error: {e}")
        finally:
            process.terminate()

# Shared condition and step tracker
condition = threading.Condition()
step = [1]  # Start at step 1

# Define operations with synchronization points
client1_operations = [
    "2",  # Login
    "sidsg",  # Username
    "Sidsg1234",  # Password
    ("4", 1, 2),  # Upload file (step 1 -> step 2)
    "/home/sid/Pictures/spike.png",  # File to upload
    ("3", 3, 4),  # List files (step 3 -> step 4)
    ("6", 5, 6),  # Rename file (step 5 -> step 6)
    "spike.png",  # Old filename
    "spike_new.png",  # New filename
]

client2_operations = [
    "2",  # Login
    "sidsg",  # Username
    "Sidsg1234",  # Password
    ("3", 2, 3),  # List files (step 2 -> step 3)
    ("7", 4, 5),  # Delete file (step 4 -> step 5)
    "spike.png",  # File to delete
]

# Start threads
thread1 = threading.Thread(target=run_client_operations, args=("Client1", 7000, client1_operations, "client1.log", condition, step))
thread2 = threading.Thread(target=run_client_operations, args=("Client2", 7000, client2_operations, "client2.log", condition, step))

thread1.start()
thread2.start()

thread1.join()
thread2.join()

print("Test completed. Logs are saved in 'client1.log' and 'client2.log'.")
