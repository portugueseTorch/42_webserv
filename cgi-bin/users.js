const deleteUser = ((event, userToDel) => {
	event.preventDefault();

	fetch('users.py', {
        method: 'DELETE',
        body: "user=" + userToDel,
		redirect: 'manual'
    })
	.then(response => {
		if (response.type === 'opaqueredirect')
			window.location.href = response.url
		else if (!response.ok) throw new Error('Check network tab')
	})
    .catch(error => {
        console.error('Error:', error);
    });
})