################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
third_party/lwip-1.4.1/src/ff.obj: C:/ti/tivaware_c_series_2_1_4_178/third_party/fatfs/src/ff.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1271/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccs1271/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --include_path="C:/Users/BGZ/workspace_v12/enet_lwip" --include_path="C:/ti/tivaware_c_series_2_1_4_178/examples/boards/ek-tm4c129exl" --include_path="C:/ti/tivaware_c_series_2_1_4_178" --include_path="C:/ti/tivaware_c_series_2_1_4_178/third_party/lwip-1.4.1/src/include" --include_path="C:/ti/tivaware_c_series_2_1_4_178/third_party/lwip-1.4.1/src/include/ipv4" --include_path="C:/ti/tivaware_c_series_2_1_4_178/third_party/lwip-1.4.1/ports/tiva-tm4c129/include" --include_path="C:/ti/tivaware_c_series_2_1_4_178/third_party/lwip-1.4.1/apps" --include_path="C:/ti/tivaware_c_series_2_1_4_178/third_party" --advice:power=all -g --gcc --define=ccs="ccs" --define=PART_TM4C129ENCPDT --define=TARGET_IS_TM4C129_RA0 --define=EK_TM4C129_BP1 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --ual --preproc_with_compile --preproc_dependency="third_party/lwip-1.4.1/src/$(basename $(<F)).d_raw" --obj_directory="third_party/lwip-1.4.1/src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


