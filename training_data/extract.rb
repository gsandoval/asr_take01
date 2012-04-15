#!/usr/bin/env ruby

classes = %w[apagar prender noise luces]
input_dir = "clean_samples"
output_dir = "features"

classes.each do |c|
	for i in (1..20)		
		input_file = "#{input_dir}\\#{c}#{i < 10? "0" : ""}#{i}.wav"
		output_file = "#{output_dir}\\#{c}#{i < 10? "0" : ""}#{i}.txt"
		system("asr_take01.exe -e #{input_file} > #{output_file}")
	end
end
