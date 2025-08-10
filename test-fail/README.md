Failing build fixture for coddle error-handling tests.

How to run
- From this directory run: `../coddle`
- Expected: compiler diagnostics on stderr, one formatted coddle error line, and a non-zero exit code.

Notes
- Generated artifacts under `.coddle/` are ignored via `.gitignore`.
