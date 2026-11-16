# Feature 9 - Collaborative Git Workflow

This document records the fork / issue / pull-request workflow required by
Feature 9. It is written as a reproducible procedure because the student does
not have a second GitHub account; the remote-side steps must be performed
manually while logged in to the secondary account.

Primary repository: `https://github.com/abdullah2k05/BSDSF24A020-OS-A03`

## 1. Prepare a secondary account

1. Create a second GitHub account with a different email address.
2. Log out of the primary account and log in to the secondary account in the
   browser.

## 2. Fork the primary repository

1. As the secondary user, open the primary repository page.
2. Click **Fork** (top right) and create the fork under the secondary account.
   The fork will be at `https://github.com/<secondary-user>/BSDSF24A020-OS-A03`.

## 3. Report a mock bug on the primary repository

1. Log back in as the primary user.
2. Open the primary repository and go to **Issues -> New issue**.
3. Use a title such as:

   > Bug: `help` command does not list the `set` command

4. Use a body such as:

   > The `set` command was added in Feature 8, but the `help` built-in was not
   > updated to include it in the list of available built-ins.

5. Submit the issue.

## 4. Fix the issue from the fork

1. As the secondary user, clone the fork to a separate directory:

   ```bash
   git clone https://github.com/<secondary-user>/BSDSF24A020-OS-A03.git \
       /tmp/BSDSF24A020-OS-A03-fork
   cd /tmp/BSDSF24A020-OS-A03-fork
   ```

2. Create a fix branch:

   ```bash
   git checkout -b fix-help-command-bug
   ```

3. Edit `src/builtins.c` so the `help` output lists `set` (it already does in
   the final version of this repository; the fix may simply confirm it).
4. Build and test:

   ```bash
   make
   printf 'help\nexit\n' | ./bin/myshell
   ```

5. Commit and push the branch to the fork:

   ```bash
   git add src/builtins.c
   git commit -m "fix: list set command in help output"
   git push -u origin fix-help-command-bug
   ```

## 5. Open the pull request

1. Open the fork page on GitHub.
2. Click **Compare & pull request** for the pushed branch.
3. Confirm the base repository is the primary repository and the base branch
   is `main`.
4. Add a description that references the issue (for example
   `Fixes #1`) and create the pull request.

## 6. Review and merge as the maintainer

1. Log back in as the primary user.
2. Open the primary repository's **Pull requests** tab.
3. Open the pull request, review **Files changed**, and merge it into `main`.
4. In the local primary repository, synchronise the merge:

   ```bash
   cd /home/abdullah/1-Data/Uni-worst-city/5th-Semester/OS/Post-Mids/Assignemnt-no4
   git pull origin main
   ```

## 7. Push all branches for grading

From the primary repository:

```bash
git push origin --all
git push origin --tags
```

The feature branches visible remotely are:

- `feature-built-ins`
- `feature-history`
- `feature-readline`
- `feature-io-redirection`
- `feature-multitasking`
- `feature-if-then-else`
- `feature-variables`
- `feature-git-workflow`

## Note

Steps 2-6 require the secondary GitHub account and browser access; they could
not be executed automatically in this environment. The repository history,
branches, tags and releases for Features 1-8 were created as required.
