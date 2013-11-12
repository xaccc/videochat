class UrlMappings {

	static mappings = {
		"/$controller/$action?/$id?"{
			constraints {
				// apply constraints here
			}
		}
        
        "/api/$action/$uid?" (controller:"api")

		"/"(view:"/index")
		"500"(view:'/error')
	}
}
