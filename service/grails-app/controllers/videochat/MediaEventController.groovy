package videochat

import org.springframework.dao.DataIntegrityViolationException

class MediaEventController {

    static allowedMethods = [save: "POST", update: "POST", delete: "POST"]

    def index() {
        redirect(action: "list", params: params)
    }

    def list(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        [mediaEventInstanceList: MediaEvent.list(params), mediaEventInstanceTotal: MediaEvent.count()]
    }

    def create() {
        [mediaEventInstance: new MediaEvent(params)]
    }

    def save() {
        def mediaEventInstance = new MediaEvent(params)
        if (!mediaEventInstance.save(flush: true)) {
            render(view: "create", model: [mediaEventInstance: mediaEventInstance])
            return
        }

        flash.message = message(code: 'default.created.message', args: [message(code: 'mediaEvent.label', default: 'MediaEvent'), mediaEventInstance.id])
        redirect(action: "show", id: mediaEventInstance.id)
    }

    def show(String id) {
        def mediaEventInstance = MediaEvent.get(id)
        if (!mediaEventInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaEvent.label', default: 'MediaEvent'), id])
            redirect(action: "list")
            return
        }

        [mediaEventInstance: mediaEventInstance]
    }

    def edit(String id) {
        def mediaEventInstance = MediaEvent.get(id)
        if (!mediaEventInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaEvent.label', default: 'MediaEvent'), id])
            redirect(action: "list")
            return
        }

        [mediaEventInstance: mediaEventInstance]
    }

    def update(String id, Long version) {
        def mediaEventInstance = MediaEvent.get(id)
        if (!mediaEventInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaEvent.label', default: 'MediaEvent'), id])
            redirect(action: "list")
            return
        }

        if (version != null) {
            if (mediaEventInstance.version > version) {
                mediaEventInstance.errors.rejectValue("version", "default.optimistic.locking.failure",
                          [message(code: 'mediaEvent.label', default: 'MediaEvent')] as Object[],
                          "Another user has updated this MediaEvent while you were editing")
                render(view: "edit", model: [mediaEventInstance: mediaEventInstance])
                return
            }
        }

        mediaEventInstance.properties = params

        if (!mediaEventInstance.save(flush: true)) {
            render(view: "edit", model: [mediaEventInstance: mediaEventInstance])
            return
        }

        flash.message = message(code: 'default.updated.message', args: [message(code: 'mediaEvent.label', default: 'MediaEvent'), mediaEventInstance.id])
        redirect(action: "show", id: mediaEventInstance.id)
    }

    def delete(String id) {
        def mediaEventInstance = MediaEvent.get(id)
        if (!mediaEventInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaEvent.label', default: 'MediaEvent'), id])
            redirect(action: "list")
            return
        }

        try {
            mediaEventInstance.delete(flush: true)
            flash.message = message(code: 'default.deleted.message', args: [message(code: 'mediaEvent.label', default: 'MediaEvent'), id])
            redirect(action: "list")
        }
        catch (DataIntegrityViolationException e) {
            flash.message = message(code: 'default.not.deleted.message', args: [message(code: 'mediaEvent.label', default: 'MediaEvent'), id])
            redirect(action: "show", id: id)
        }
    }
}
