################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
CC3100SupportPackage/board/%.obj: ../CC3100SupportPackage/board/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs910/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccs910/ccs/ccs_base/arm/include" --include_path="C:/Users/Danny/workspace_v9/lab5/G8RTOS" --include_path="C:/Users/Danny/workspace_v9/lab5/CC3100SupportPackage/board" --include_path="C:/Users/Danny/workspace_v9/lab5/CC3100SupportPackage/cc3100_usage" --include_path="C:/Users/Danny/workspace_v9/lab5/CC3100SupportPackage/simplelink/include" --include_path="C:/Users/Danny/workspace_v9/lab5/CC3100SupportPackage/simplelink/source" --include_path="C:/Users/Danny/workspace_v9/lab5/CC3100SupportPackage/SL_Common" --include_path="C:/Users/Danny/workspace_v9/lab5/CC3100SupportPackage/spi_cc3100" --include_path="C:/Users/Danny/workspace_v9/lab5/BoardSupportPackage/DriverLib" --include_path="C:/Users/Danny/workspace_v9/lab5/BoardSupportPackage/inc" --include_path="C:/Users/Danny/workspace_v9/lab5/BoardSupportPackage/src" --include_path="C:/ti/ccs910/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Danny/workspace_v9/lab5" --include_path="C:/Users/Danny/Desktop/UF/uP2/uP2 from daniel/Lab3/G8RTOS_Empty_Lab3/G8RTOS_Empty_Lab3" --include_path="C:/Users/Danny/Downloads" --include_path="C:/ti/ccs910/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="CC3100SupportPackage/board/$(basename $(<F)).d_raw" --obj_directory="CC3100SupportPackage/board" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


