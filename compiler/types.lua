types = {
	["num"] = {
		["methods"] = {
		},
	},
	["sample"] = {
		["methods"] = {
			{
				["name"] = "ADSR",
				["id"] = kOpNodeADSR,
				["arguments"] = {"num", "num", "num", "num"},
				["return_type"] = "sample",
				["description"] = "Basic ADSR envelope.",
			},
		},
	},
	["spectrum"] = {
		["methods"] = {
			{
				["name"] = "addSaw",
				["id"] = kOpAddSaw,
				["arguments"] = {"num", "num"},
				["return_type"] = "spectrum",
				["description"] = "Adds saw to spectrum. Arguments: (num harmonic, num gain)",
			},
		},
	},
}
