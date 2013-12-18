package videochat



import org.junit.*
import grails.test.mixin.*

@TestFor(MediaEventController)
@Mock(MediaEvent)
class MediaEventControllerTests {

    def populateValidParams(params) {
        assert params != null
        // TODO: Populate valid properties like...
        //params["name"] = 'someValidName'
    }

    void testIndex() {
        controller.index()
        assert "/mediaEvent/list" == response.redirectedUrl
    }

    void testList() {

        def model = controller.list()

        assert model.mediaEventInstanceList.size() == 0
        assert model.mediaEventInstanceTotal == 0
    }

    void testCreate() {
        def model = controller.create()

        assert model.mediaEventInstance != null
    }

    void testSave() {
        controller.save()

        assert model.mediaEventInstance != null
        assert view == '/mediaEvent/create'

        response.reset()

        populateValidParams(params)
        controller.save()

        assert response.redirectedUrl == '/mediaEvent/show/1'
        assert controller.flash.message != null
        assert MediaEvent.count() == 1
    }

    void testShow() {
        controller.show()

        assert flash.message != null
        assert response.redirectedUrl == '/mediaEvent/list'

        populateValidParams(params)
        def mediaEvent = new MediaEvent(params)

        assert mediaEvent.save() != null

        params.id = mediaEvent.id

        def model = controller.show()

        assert model.mediaEventInstance == mediaEvent
    }

    void testEdit() {
        controller.edit()

        assert flash.message != null
        assert response.redirectedUrl == '/mediaEvent/list'

        populateValidParams(params)
        def mediaEvent = new MediaEvent(params)

        assert mediaEvent.save() != null

        params.id = mediaEvent.id

        def model = controller.edit()

        assert model.mediaEventInstance == mediaEvent
    }

    void testUpdate() {
        controller.update()

        assert flash.message != null
        assert response.redirectedUrl == '/mediaEvent/list'

        response.reset()

        populateValidParams(params)
        def mediaEvent = new MediaEvent(params)

        assert mediaEvent.save() != null

        // test invalid parameters in update
        params.id = mediaEvent.id
        //TODO: add invalid values to params object

        controller.update()

        assert view == "/mediaEvent/edit"
        assert model.mediaEventInstance != null

        mediaEvent.clearErrors()

        populateValidParams(params)
        controller.update()

        assert response.redirectedUrl == "/mediaEvent/show/$mediaEvent.id"
        assert flash.message != null

        //test outdated version number
        response.reset()
        mediaEvent.clearErrors()

        populateValidParams(params)
        params.id = mediaEvent.id
        params.version = -1
        controller.update()

        assert view == "/mediaEvent/edit"
        assert model.mediaEventInstance != null
        assert model.mediaEventInstance.errors.getFieldError('version')
        assert flash.message != null
    }

    void testDelete() {
        controller.delete()
        assert flash.message != null
        assert response.redirectedUrl == '/mediaEvent/list'

        response.reset()

        populateValidParams(params)
        def mediaEvent = new MediaEvent(params)

        assert mediaEvent.save() != null
        assert MediaEvent.count() == 1

        params.id = mediaEvent.id

        controller.delete()

        assert MediaEvent.count() == 0
        assert MediaEvent.get(mediaEvent.id) == null
        assert response.redirectedUrl == '/mediaEvent/list'
    }
}
