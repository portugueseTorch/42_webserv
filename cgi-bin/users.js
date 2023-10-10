const deleteUser = ((event, userToDel) => {
	event.preventDefault();

	fetch('users.py', {
        method: 'DELETE',
        body: "user=" + userToDel
    })
	.then(response => {
		if (!response.ok){
            throw new Error('Network response was not ok');
        }
		return response;
	})
	.then(response => response.text())
    .then(data => {
		document.body.innerHTML = data;
	})
    .catch(error => {
        console.error('Error:', error);
    });
})