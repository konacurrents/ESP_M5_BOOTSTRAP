# show the options

compileM5Atom:
	/opt/homebrew/bin/arduino-cli compile --build-property compiler.cpp.extra_flags=-DESP_M5_ATOM_LITE --fqbn esp32:esp32:m5stick-c | tee output

# uploads this to the OTA/Bootstrap
# This requires premission to upload to a foreign web location
uploadBootstrap:
	scp -i ~/.ssh/idogwatch_001.pem ESP_M5_BOOTSTRAP.ino.m5stack_stickc_plus.bin ec2-user@knowledgeshark.me:html/KnowledgeShark.org/OTA/Bootstrap
	mv ESP_M5_BOOTSTRAP.ino.m5stack_stickc_plus.bin ESP_M5_BOOTSTRAP.ino.m5stack_stickc_plus.bin.save
