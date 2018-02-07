# naprom
Experimenting with Node N-API async promises.

1. Creates a promise and returns it from C land.
2. Queues and runs some async work ... a wait.
3. Completes the async work and resolves the promise, keeping a reference to a string alive.

Run with

    npm install
    node index.js
