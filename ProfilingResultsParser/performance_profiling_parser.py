import generate_charts_from_log as logs
import generate_charts_from_per_scenario as scenarios


SCENARIO_COUNT = 10

#logs.generate_charts_from_log('./Data/CompressedMesherDemo.log', SCENARIO_COUNT)
scenarios.generate_charts_interate_logs('./Data/PerScenario', SCENARIO_COUNT)