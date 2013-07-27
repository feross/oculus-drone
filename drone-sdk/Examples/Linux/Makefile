
.PHONY:

all:
	@$(MAKE) -C ../../ARDroneLib/Soft/Build USE_LINUX=yes
	@$(MAKE) -C Navigation/Build USE_LINUX=yes 
	@$(MAKE) -C sdk_demo/Build USE_LINUX=yes 
#	@$(MAKE) -C Testbenches/multiconfiguration/Build USE_LINUX=yes
	@$(MAKE) -C Testbenches/ftp_test/Build USE_LINUX=yes

$(MAKECMDGOALS):
	@$(MAKE) -C ../../ARDroneLib/Soft/Build USE_LINUX=yes $(MAKECMDGOALS)
	@$(MAKE) -C Navigation/Build USE_LINUX=yes $(MAKECMDGOALS)
	@$(MAKE) -C sdk_demo/Build USE_LINUX=yes $(MAKECMDGOALS)
#	@$(MAKE) -C Testbenches/multiconfiguration/Build USE_LINUX=yes $(MAKECMDGOALS)
	@$(MAKE) -C Testbenches/ftp_test/Build USE_LINUX=yes $(MAKECMDGOALS)


