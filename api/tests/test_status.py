#!/usr/bin/python3
"""Status check test.
"""


def test_status_check(client):
    """Test the status check endpoint.
    """
    res = client.get('/api/v1/status')
    assert res.status_code == 200
    assert res.json['status'] == 'OK'
