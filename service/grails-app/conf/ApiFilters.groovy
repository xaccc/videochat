class ApiFilters {
    def filters = {
        all(controller:'api', action:'*') {
            before = {
                response.setHeader('Cache-Control', 'no-cache')
                response.setHeader('Expires', '-1')
            }
        }
    }
}