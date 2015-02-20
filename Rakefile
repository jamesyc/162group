# coding: utf-8

current_proj = "proj1"
proj_dir = "pintos/src"
current_ch = "checkpoint1"

desc "Submit assignment"
task :default do
    puts "Creating release branch if one does not already exist"
    system("git branch release/#{current_proj}/#{current_ch}")

    puts "Cleaning executables"
    clean = system("make clean -C #{proj_dir}")

    puts "Switching to release branch"
    submit_checkout = system("git checkout release/#{current_proj}/#{current_ch}")

    puts "Merging files into release branch"
    merge = system("git merge master")

    puts "Pushing to release branch on remote"
    push = system("git push -f group release/#{current_proj}/#{current_ch}")

    puts "Switching back to master branch"
    master_checkout = system("git checkout master")

    puts (clean and submit_checkout and merge and push and master_checkout) ? "Success" : "Failed"
end


