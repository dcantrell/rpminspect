# Source Code Control Notes

### Signed-off-by

Using

```sh
git commit -s
```

to sign-off on commits is preferred, but not required.  See
https://developercertificate.org/ for more information.

### Short Log Headers

To categorize commits and make release log generation easier, please
use categorization headers on the first line of git commit messages.
The headers the project uses are:

| Prefix | Description |
| ------ | ----------- |
| [bld]  | General release and build process changes |
| [tst]  | Test suite commits |
| [fix]  | General bug fix in the library or frontend program |
| [lib]  | librpminspect feature or significant change |
| [new]  | New inspections or inspection changes (not bug fixes) |
| [cmd]  | rpminspect(1) changes or improvements related to it |
| [cfg]  | Config file or data/ file changes |
| [ci]   | Changes to the GitHub Actions CI scripts and files |
| [doc]  | Documentation changes |

This list may expand over time.

NOTE: Short log messages without a header like this will be excluded
from release announcements.  That may be appropriate for some commits.

### Consolidated Project History

Sending pull requests is the preferred workflow, which means
contributors need to track the upstream repo in their forked copies.
Please avoid merge commits as you update your forks so that the commit
history in the main project is consolidated.  An easy way to do that
is:

```sh
git checkout master
git remote add upstream https://github.com/dcantrell/rpminspect
git fetch upstream
git rebase upstream/master
git push -f
```

This will track upstream and rebase your copy to the upstream copy.
The force push is required to your copy since you are rewriting the
project history to match upstream.  You need to do this on a clean
repo, so stash anything you are working on and ensure your copy is
clean.

### AI Policy

As a policy, the rpminspect project does not accept use of generative
AI in contributions.
