# coding: utf-8

# Configuration settings

# These variables should be updated every time a checkpoint deadline is
# reached, or a new project is released.

config = {
  :project => "proj2", 
  :checkpoint => "checkpoint2", 
  :dir => "pintos/src", 
  :remote => "group", 
  :branch => {
    :master => "master", 
    :autograder => "ag", 
    :release => "release"
  }
}


# Colorize terminal output

module Colors
  def colorize(text, color_code)
  "\033[#{color_code}m#{text}\033[0m"
  end

  {
  :black    => 30,
  :red      => 31,
  :green    => 32,
  :yellow   => 33,
  :blue     => 34,
  :magenta  => 35,
  :cyan     => 36,
  :white    => 37
  }.each do |key, color_code|
  define_method key do |text|
    colorize(text, color_code)
  end
  end
end

include Colors


# Rake tasks

desc "Submit assignment"
task :push, [:branch, :project, :checkpoint, :dir] do |t, args|
  args.with_defaults(:branch => config[:branch][:autograder], 
                     :project => config[:project], 
                     :checkpoint => config[:checkpoint], 
                     :dir => config[:dir])

  master_branch = "#{args.project}/#{config[:branch][:master]}"
  staff_branch = "#{args.branch}/#{args.project}/#{args.checkpoint}"

  begin
    system("git branch -D #{staff_branch}")
    system("make clean -C #{args.dir}") or raise
    system("git branch #{staff_branch}") or raise
    system("git checkout #{staff_branch}") or raise
    system("git merge #{master_branch}") or raise
    system("git push -f #{config[:remote]} #{staff_branch}") or raise
    system("git checkout #{master_branch}") or raise
    puts green("Submitted successfully")
  rescue
    system("git reset")
    system("git checkout #{master_branch}")
    system("git branch -D #{staff_branch}")
    puts red("An error occurred while submitting, rolled back changes")
  end
end


desc "Submit assignment to autograder"
task :ag, [:branch, :proj, :checkpoint, :dir] do |t, args|
  Rake::Task[:push].invoke(config[:branch][:release])
  puts cyan "Submitted the project! Sit back and relax."
end


desc "Submit assignment to autograder"
task :ag, [:branch, :proj, :checkpoint, :dir] do |t, args|
  Rake::Task[:push].invoke(config[:branch][:autograder])
  puts magenta "Don't forget to push to release! #{red("(rake release)")}"
end


desc "Default task"
task :default => [:ag] do
endoint}")
    args.with_defaults(proj_dir: "#{proj_dir}")

    Rake::Task[:push].invoke(args.branch, args.proj, args.checkpoint, args.proj_dir)
end


