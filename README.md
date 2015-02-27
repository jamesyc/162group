CS162: Operating Systems and Systems Programming
================================================

### Overview

This is a private repository for all group project files and submissions. 


### Submissions

An automated Rakefile script is proved to streamline submission of projects to both the autograder and release branches. First, ensure that Rake is installed on the Vagrant box by running:

```bash
sudo apt-get install rake
```

Check the Rakefile to make sure that all the variables in the file header are set correctly. Running `rake` will submit a project to the autograder by default. In order to submit the final project to the `release` branch, execute `rake release`.


### Setting up Vagrant

1. Run `vagrant init`
2. Forward private key using `ssh-add ~/.ssh/id_rsa`
3. Run `vagrant up`, then `vagrant provision` if necessary
4. SSH into the box with `vagrant ssh`.
