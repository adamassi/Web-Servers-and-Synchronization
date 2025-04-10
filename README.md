# 🌐 Multi-threaded Web Server with Synchronization and Scheduling

## 📝 Overview

This project is an implementation of a multi-threaded web server in C, completed for the Technion's **"Web Servers and Synchronization"** course (HW3). The server handles concurrent HTTP GET requests and serves both static 🗂️ and dynamic ⚙️ content. Starting from a basic single-threaded web server, the project adds advanced features like thread pooling, request queuing, scheduling algorithms, and usage statistics 📊.

---

## 🚀 Features Implemented

### 🔁 1. Multi-threading
- Transformed the original server into a **multi-threaded** version using **POSIX threads** 🧵.
- Created a **thread pool** that handles incoming HTTP requests concurrently.
- The **main thread** accepts connections and enqueues them 📬.
- **Worker threads** dequeue and process requests 🛠️.

### 🧩 2. Synchronization
- Used `pthread_mutex` for locking 🔒 and `pthread_cond` for signaling 🔔.
- Prevented race conditions and ensured thread-safe queue operations.
- Blocked producer/consumer threads properly without busy-waiting 🧘‍♂️.

### ⚖️ 3. Overload Scheduling Policies
Handled request overflow with different strategies:

- 🧱 **block** – Wait until space is available.
- ✂️ **drop_tail (dt)** – Drop the newest request.
- 🗑️ **drop_head (dh)** – Drop the oldest queued request.
- 🚫 **block_flush (bf)** – Wait until queue is empty, then drop.
- 🎲 **drop_random (random)** – (Bonus) Drop 50% of waiting requests randomly.

### 🧨 4. Special Suffix: `.skip`
- For requests ending in `.skip`, the server dequeues and defers handling the **last request** in the queue 🔄.
- Ensured correct queue accounting while applying the `.skip` logic.

### 📈 5. Usage Statistics
Embedded statistics in HTTP response headers:

🕒 **Per Request:**
- `Stat-Req-Arrival` – Time of arrival.
- `Stat-Req-Dispatch` – Wait time in queue.

🧵 **Per Thread:**
- `Stat-Thread-Id` – Thread index.
- `Stat-Thread-Count` – Total requests handled.
- `Stat-Thread-Static` – Static requests count.
- `Stat-Thread-Dynamic` – Dynamic requests count.

Headers are visible via custom clients or tools like `curl` 🛠️.

---

## ⚙️ How to Run

```bash
make
./server [port] [num_threads] [queue_size] [schedalg]

